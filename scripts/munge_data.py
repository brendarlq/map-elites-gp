import numpy as np
import sys
import pandas as pd
import glob
import os

# This script expects two command line arguments:
#   - a glob-style pattern indicating which directories to analyze
#         (remember to put quotation marks around it)
#   - the prefix filename to store data in (main data fill get stored in filename.csv,
#     trait data will get stored in filename_traits.csv)

# It will extract the path (x, y, and z coordinates) taken by the fittest
# lineage from the phylogeny_5000.csv file from each directory matched by
# the glob pattern provided in the first command-line argument.

def main():
    glob_pattern = sys.argv[1]
    outfilename = sys.argv[2]
    frames = []
    trait_frames = []

	# print(glob_pattern, glob.glob(glob_pattern))

    for dirname in glob.glob(glob_pattern):
        run_log = dirname + "/run.log"
        if not (os.path.exists(run_log)):
            print("run log not found")
            continue

        local_data = {}

        with open(run_log) as run_log_file:
            for line in run_log_file:
                if line.startswith("Doing initial"):
                    break
                elif not line.startswith("set"):
                    continue
                line = line.split()
                local_data[line[1]] = line[2]

		# Create function

        temp = pd.DataFrame()
        for k in local_data:
            temp[k] = local_data[k]

        fitness_df = pd.read_csv(dirname+"/fitness.csv", index_col="update")
        systematics_df = pd.read_csv(dirname+"/systematics.csv", index_col="update")
        population_df = pd.read_csv(dirname+"/population.csv", index_col="update")
        trait_df = pd.read_csv(dirname+"/traits.dat", index_col="update")
        # phylodiversity_df = pd.read_csv(dirname+"/phylodiversity.csv", index_col="update")
        # dominant_df = pd.read_csv(dirname+"/dominant.csv", index_col="update")
        # lin_df = pd.read_csv(dirname+"/lineage_mutations.csv", index_col="update")

        df = pd.concat([fitness_df, systematics_df, population_df], axis=1)
        frames.append(pd.concat([df, temp]))
        trait_frames.append(pd.concat[trait_df, temp])

    all_data = pd.concat(frames)
    trait_data = pd.concat(trait_frames)
    all_data.to_csv(outfilename+".csv",index=False)
    trait_data.to_csv(outfilename+"_traits.csv", index=False)

	#print(df)
	# Evaluate :-)
	# x = np.ones(2)
	# value = f.evaluate(x)




if __name__ == "__main__":
	main()
