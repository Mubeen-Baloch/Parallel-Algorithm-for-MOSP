# extract_summary.py
# Aggregates iteration stats and memory usage from all output_rankX.txt files

import os
import re
import glob
import csv

output_dir = "results"
out_pattern = os.path.join(output_dir, "output_rank*.txt")
summary_file = os.path.join(output_dir, "summary.csv")

summary = []

for path in sorted(glob.glob(out_pattern)):
    rank = int(re.search(r'rank(\d+)', path).group(1))
    total_updates = 0
    last_iter = 0
    max_rss = 0
    vertices = 0

    with open(path, 'r') as f:
        for line in f:
            if "Iteration" in line:
                last_iter += 1
                m = re.search(r"Total updates = (\d+)", line)
                if m:
                    total_updates += int(m.group(1))
            elif "RSS" in line:
                m = re.search(r"RSS: (\d+)", line)
                if m:
                    max_rss = max(max_rss, int(m.group(1)))
            elif line.startswith("Process") and ":" in line:
                vertices += 1

    summary.append([rank, last_iter, total_updates, max_rss, vertices])

with open(summary_file, 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(["Rank", "Iterations", "Total Updates", "Max RSS (MB)", "Vertices Owned"])
    writer.writerows(summary)

print(f"Summary written to {summary_file}")

