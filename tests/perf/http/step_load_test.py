#!/usr/bin/env python3

import argparse
import json
import math
import subprocess
import sys
from pathlib import Path


def _run(cmd: list[str]) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, check=False, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)


def _load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def _find_results_dir(output: str) -> Path | None:
    marker = "[OK] results written to: "
    for line in output.splitlines():
        if line.startswith(marker):
            return Path(line[len(marker) :].strip())
    return None


def _fmt(v: float, digits: int = 2) -> str:
    if v is None or (isinstance(v, float) and (math.isnan(v) or math.isinf(v))):
        return "-"
    return f"{v:.{digits}f}"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--conf", default="bin/config/gateway_http")
    ap.add_argument("--url", default="http://127.0.0.1:8080/ping")
    ap.add_argument("--threads", type=int, default=8)
    ap.add_argument("--duration", type=int, default=15)
    ap.add_argument("--warmup", type=int, default=3)
    ap.add_argument(
        "--connections",
        default="512,1024,2048,4096,8192,10240",
        help="comma-separated list",
    )
    ap.add_argument("--label", default="step_load")
    args = ap.parse_args()

    repo_root = Path(__file__).resolve().parents[3]
    runner = repo_root / "tests/perf/http/run_gateway_http_wrk.py"

    conf_path = Path(args.conf)
    if not conf_path.is_absolute():
        conf_path = (repo_root / conf_path).resolve()
    args.conf = str(conf_path)

    conns = [int(x) for x in args.connections.split(",") if x.strip()]

    rows: list[dict] = []
    print(
        f"Running step load test: url={args.url} threads={args.threads} duration={args.duration}s conf={args.conf}"
    )

    for c in conns:
        label = f"{args.label}_c{c}"
        cmd = [
            sys.executable,
            str(runner),
            "--label",
            label,
            "--conf",
            args.conf,
            "--url",
            args.url,
            "--threads",
            str(args.threads),
            "--connections",
            str(c),
            "--duration",
            str(args.duration),
            "--warmup",
            str(args.warmup),
            "--kill-existing",
        ]
        print(f"\n[RUN] connections={c}")
        proc = _run(cmd)
        if proc.returncode != 0:
            print(proc.stdout)
            print(f"[FAIL] runner exit={proc.returncode}")
            break

        out_dir = _find_results_dir(proc.stdout)
        if not out_dir:
            print(proc.stdout)
            print("[FAIL] cannot locate results dir")
            break

        metrics = _load_json(out_dir / "metrics.json")

        qps = float(metrics.get("requests_per_sec", 0.0) or 0.0)
        avg_ms = float(metrics.get("latency_avg_ms", 0.0) or 0.0)
        p99_ms = float((metrics.get("latency_distribution_ms", {}) or {}).get("99", 0.0) or 0.0)
        errs = int(metrics.get("error_count", 0) or 0)

        est_ok = qps * float(args.duration)
        total_attempts = est_ok + float(errs)
        err_rate = (float(errs) / total_attempts) if total_attempts > 0 else 0.0

        rows.append(
            {
                "connections": c,
                "qps": qps,
                "avg_ms": avg_ms,
                "p99_ms": p99_ms,
                "errors": errs,
                "error_rate": err_rate,
                "results_dir": str(out_dir),
            }
        )

        print(
            f"[OK] qps={_fmt(qps)} avg={_fmt(avg_ms)}ms p99={_fmt(p99_ms)}ms errors={errs} error_rate={_fmt(err_rate*100, 2)}%"
        )

    print("\n" + "=" * 100)
    print(f"{'Conn':>6}  {'QPS':>10}  {'Avg(ms)':>10}  {'P99(ms)':>10}  {'Errors':>10}  {'Err%':>8}")
    print("-" * 100)
    for r in rows:
        print(
            f"{r['connections']:>6}  {r['qps']:>10.2f}  {r['avg_ms']:>10.2f}  {r['p99_ms']:>10.2f}  {r['errors']:>10}  {r['error_rate']*100:>7.2f}%"
        )
    print("=" * 100)

    out_json = repo_root / "tests/perf/results" / f"{args.label}_summary.json"
    out_json.write_text(json.dumps({"args": vars(args), "rows": rows}, indent=2), encoding="utf-8")
    print(f"Summary written: {out_json}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
