#!/usr/bin/env python3
"""
Capture MCU IMU telemetry for static and dynamic gyro tests.

Firmware line format:
  $I,<ts_ms>,<pitch_deg>,<roll_deg>,<yaw_deg>,<gx_dps>,<gy_dps>,<gz_dps>,<ax_g>,<ay_g>,<az_g>,<ready>
"""
from __future__ import annotations

import argparse
import csv
import datetime as dt
import math
import signal
import statistics
import sys
import time
from pathlib import Path

import serial

STOP = False

CSV_HEADER = [
    "host_ts",
    "ts_ms",
    "pitch_deg",
    "roll_deg",
    "yaw_deg",
    "gx_dps",
    "gy_dps",
    "gz_dps",
    "ax_g",
    "ay_g",
    "az_g",
    "accel_norm_g",
    "ready",
    "raw_line",
]


def on_sigint(*_args) -> None:
    global STOP
    STOP = True


def send_cmd(ser: serial.Serial, text: str) -> None:
    if not text.endswith("\n"):
        text += "\r\n"
    ser.write(text.encode("ascii"))
    ser.flush()


def parse_imu_line(line: str) -> dict | None:
    if not line.startswith("$I,"):
        return None

    parts = line.split(",")
    if len(parts) < 12:
        return None

    try:
        ax = float(parts[8])
        ay = float(parts[9])
        az = float(parts[10])
        return {
            "ts_ms": int(parts[1]),
            "pitch_deg": float(parts[2]),
            "roll_deg": float(parts[3]),
            "yaw_deg": float(parts[4]),
            "gx_dps": float(parts[5]),
            "gy_dps": float(parts[6]),
            "gz_dps": float(parts[7]),
            "ax_g": ax,
            "ay_g": ay,
            "az_g": az,
            "accel_norm_g": math.sqrt(ax * ax + ay * ay + az * az),
            "ready": int(parts[11]),
            "raw_line": line,
        }
    except ValueError:
        return None


def row_from_sample(sample: dict) -> list:
    return [
        f"{time.time():.6f}",
        sample["ts_ms"],
        sample["pitch_deg"],
        sample["roll_deg"],
        sample["yaw_deg"],
        sample["gx_dps"],
        sample["gy_dps"],
        sample["gz_dps"],
        sample["ax_g"],
        sample["ay_g"],
        sample["az_g"],
        sample["accel_norm_g"],
        sample["ready"],
        sample["raw_line"],
    ]


def mean(values: list[float]) -> float:
    return statistics.fmean(values) if values else 0.0


def pstdev(values: list[float]) -> float:
    return statistics.pstdev(values) if len(values) >= 2 else 0.0


def span(values: list[float]) -> float:
    return (max(values) - min(values)) if values else 0.0


def integrate_rate(samples: list[dict], key: str) -> float:
    angle = 0.0
    for prev, cur in zip(samples, samples[1:]):
        dt_s = (cur["ts_ms"] - prev["ts_ms"]) / 1000.0
        if dt_s <= 0.0 or dt_s > 1.0:
            continue
        angle += 0.5 * (prev[key] + cur[key]) * dt_s
    return angle


def format_axis_stats(name: str, values: list[float], unit: str) -> list[str]:
    return [
        f"{name}_mean_{unit}: {mean(values):.6f}",
        f"{name}_std_{unit}: {pstdev(values):.6f}",
        f"{name}_min_{unit}: {min(values):.6f}" if values else f"{name}_min_{unit}: 0.000000",
        f"{name}_max_{unit}: {max(values):.6f}" if values else f"{name}_max_{unit}: 0.000000",
        f"{name}_span_{unit}: {span(values):.6f}",
    ]


def summarize(samples: list[dict], mode: str) -> str:
    if not samples:
        return "no IMU samples captured\n"

    ts0 = samples[0]["ts_ms"]
    ts1 = samples[-1]["ts_ms"]
    duration_s = max(0.0, (ts1 - ts0) / 1000.0)
    rate_hz = ((len(samples) - 1) / duration_s) if duration_s > 0.0 and len(samples) > 1 else 0.0

    gx = [s["gx_dps"] for s in samples]
    gy = [s["gy_dps"] for s in samples]
    gz = [s["gz_dps"] for s in samples]
    ax = [s["ax_g"] for s in samples]
    ay = [s["ay_g"] for s in samples]
    az = [s["az_g"] for s in samples]
    an = [s["accel_norm_g"] for s in samples]
    pitch = [s["pitch_deg"] for s in samples]
    roll = [s["roll_deg"] for s in samples]
    yaw = [s["yaw_deg"] for s in samples]

    lines = [
        f"mode: {mode}",
        f"samples: {len(samples)}",
        f"mcu_duration_s: {duration_s:.3f}",
        f"estimated_rate_hz: {rate_hz:.3f}",
        f"ready_samples: {sum(1 for s in samples if s['ready'])}",
        "",
        "[gyro dps]",
        *format_axis_stats("gx", gx, "dps"),
        *format_axis_stats("gy", gy, "dps"),
        *format_axis_stats("gz", gz, "dps"),
        f"gx_integral_deg: {integrate_rate(samples, 'gx_dps'):.6f}",
        f"gy_integral_deg: {integrate_rate(samples, 'gy_dps'):.6f}",
        f"gz_integral_deg: {integrate_rate(samples, 'gz_dps'):.6f}",
        "",
        "[accel g]",
        *format_axis_stats("ax", ax, "g"),
        *format_axis_stats("ay", ay, "g"),
        *format_axis_stats("az", az, "g"),
        *format_axis_stats("accel_norm", an, "g"),
        "",
        "[attitude deg]",
        *format_axis_stats("pitch", pitch, "deg"),
        *format_axis_stats("roll", roll, "deg"),
        *format_axis_stats("yaw", yaw, "deg"),
        f"yaw_delta_deg: {(yaw[-1] - yaw[0]) if yaw else 0.0:.6f}",
    ]

    if mode == "static":
        lines.extend([
            "",
            "[static test hints]",
            "gyro_mean is zero-rate bias; gyro_std is short-term noise.",
            "accel_norm_mean should be close to 1 g when still.",
            "yaw_delta/gz_integral show drift during the static window.",
        ])
    else:
        lines.extend([
            "",
            "[dynamic test hints]",
            "gyro_span/max show motion excitation strength.",
            "gz_integral_deg is useful for a turntable or known-angle yaw test.",
            "compare yaw_delta_deg with gz_integral_deg to spot filter drift.",
        ])

    return "\n".join(lines) + "\n"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", default="/dev/ttyACM0")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--rate", type=int, default=100, help="MCU IMU output rate in Hz")
    ap.add_argument("--duration", type=float, default=30.0, help="capture seconds; 0 means until Ctrl-C")
    ap.add_argument("--discard-s", type=float, default=1.0, help="discard initial settling seconds")
    ap.add_argument("--mode", choices=("static", "dynamic"), default="static")
    ap.add_argument("--out-dir", default=str(Path(__file__).resolve().parent / "logs"))
    ap.add_argument("--no-init-cmds", action="store_true", help="do not send RATE/RAWLINE/PAUSE/RAWIMU")
    ap.add_argument("--keep-telem", action="store_true", help="do not send PAUSE; other telemetry may share UART")
    ap.add_argument("--resume-after", action="store_true", help="send RESUME before exit")
    args = ap.parse_args()

    signal.signal(signal.SIGINT, on_sigint)

    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    stamp = dt.datetime.now().strftime("%Y%m%d_%H%M%S")
    csv_path = out_dir / f"imu_{args.mode}_{stamp}.csv"
    summary_path = out_dir / f"imu_{args.mode}_{stamp}.summary.txt"

    try:
        ser = serial.serial_for_url(args.port, args.baud, timeout=0.2)
    except Exception as exc:
        print(f"[imu] open failed: {exc}", file=sys.stderr)
        return 1

    if not args.no_init_cmds:
        time.sleep(0.05)
        send_cmd(ser, f"$RATE,{args.rate}")
        send_cmd(ser, "$RAWLINE,0")
        if not args.keep_telem:
            send_cmd(ser, "$PAUSE")
        send_cmd(ser, "$RAWIMU,1")

    print(f"[imu] port={args.port} baud={args.baud} rate={args.rate}Hz mode={args.mode}")
    print(f"[imu] csv={csv_path}")
    print(f"[imu] summary={summary_path}")
    if args.discard_s > 0:
        print(f"[imu] discarding first {args.discard_s:.1f}s")

    start_host = time.monotonic()
    first_sample_host: float | None = None
    samples: list[dict] = []
    raw_count = 0
    last_status = 0.0

    try:
        with csv_path.open("w", newline="") as csv_f:
            writer = csv.writer(csv_f)
            writer.writerow(CSV_HEADER)

            while not STOP:
                if args.duration > 0.0 and (time.monotonic() - start_host) >= args.duration:
                    break

                raw = ser.readline()
                if not raw:
                    continue
                line = raw.decode("ascii", errors="ignore").strip()
                sample = parse_imu_line(line)
                if sample is None:
                    continue

                now = time.monotonic()
                if first_sample_host is None:
                    first_sample_host = now

                if (now - first_sample_host) < args.discard_s:
                    continue

                raw_count += 1
                writer.writerow(row_from_sample(sample))
                samples.append(sample)

                if now - last_status >= 1.0:
                    last_status = now
                    print(f"\r[imu] samples={len(samples)} ready={sample['ready']} "
                          f"gx={sample['gx_dps']:.3f} gy={sample['gy_dps']:.3f} "
                          f"gz={sample['gz_dps']:.3f} yaw={sample['yaw_deg']:.2f}   ",
                          end="")
                    sys.stdout.flush()
    finally:
        if not args.no_init_cmds:
            try:
                send_cmd(ser, "$RAWIMU,0")
                if args.resume_after:
                    send_cmd(ser, "$RESUME")
            except Exception:
                pass
        ser.close()

    print()
    summary = summarize(samples, args.mode)
    summary_path.write_text(summary, encoding="utf-8")
    print(summary)
    return 0 if raw_count > 0 else 2


if __name__ == "__main__":
    raise SystemExit(main())
