#!/bin/sh
# Print a cmake/b2 -j count from this machine's CPU and RAM.
# Override with FB_CMAKE_JOBS. Tune RAM/job with FB_CMAKE_JOB_KB (KiB).

set -eu

ncpu=$(nproc)
total_kb=$(awk '/^MemTotal:/{print $2}' /proc/meminfo)
avail_kb=$(awk '/^MemAvailable:/{print $2}' /proc/meminfo)
if [ -z "${avail_kb:-}" ] || [ "$avail_kb" -eq 0 ]; then
    avail_kb=$total_kb
fi

cgroup_kb=""
if [ -r /sys/fs/cgroup/memory.max ]; then
    v=$(cat /sys/fs/cgroup/memory.max)
    if [ "$v" != "max" ]; then
        cgroup_kb=$((v / 1024))
    fi
elif [ -r /sys/fs/cgroup/memory/memory.limit_in_bytes ]; then
    v=$(cat /sys/fs/cgroup/memory/memory.limit_in_bytes)
    if [ "$v" -gt 0 ] && [ "$v" -lt 9223372036854775807 ]; then
        cgroup_kb=$((v / 1024))
    fi
fi
if [ -n "$cgroup_kb" ] && [ "$cgroup_kb" -lt "$avail_kb" ]; then
    avail_kb=$cgroup_kb
    total_kb=$cgroup_kb
fi

reserve_kb=$((total_kb * 15 / 100))
min_reserve_kb=$((2 * 1024 * 1024))
if [ "$reserve_kb" -lt "$min_reserve_kb" ]; then
    reserve_kb=$min_reserve_kb
fi

per_job_kb=${FB_CMAKE_JOB_KB:-$((4 * 1024 * 1024))}
build_type=${BUILD_TYPE:-}
case "$build_type" in
    [Dd]ebug)
        per_job_kb=${FB_CMAKE_JOB_KB:-$((6 * 1024 * 1024))}
        ;;
esac

budget_kb=$((avail_kb - reserve_kb))
if [ "$budget_kb" -lt "$per_job_kb" ]; then
    jobs=1
else
    jobs=$((budget_kb / per_job_kb))
fi
if [ "$jobs" -gt "$ncpu" ]; then
    jobs=$ncpu
fi
if [ "$jobs" -lt 1 ]; then
    jobs=1
fi
if [ -n "${FB_CMAKE_JOBS:-}" ]; then
    jobs=$FB_CMAKE_JOBS
fi

avail_gb=$((avail_kb / 1024 / 1024))
per_job_gb=$((per_job_kb / 1024 / 1024))
echo "fb-cmake-jobs: ${jobs} (nproc=${ncpu} avail=${avail_gb}GiB per_job=${per_job_gb}GiB type=${build_type:-release})" >&2
echo "$jobs"
