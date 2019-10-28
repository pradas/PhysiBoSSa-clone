from scipy.io import loadmat
import pandas as pd
import argparse
import os

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="This programs creates a macrocomplex given the pair of interactions.")
    parser.add_argument('-i', '--input',
                        dest="input",
                        action="store",
                        required=True,
                        help="The input folder containing the complexes files")
    parser.add_argument('-o', '--output',
                        dest="output",
                        action="store",
                        default=".output/",
                        help="The output file")

    options = parser.parse_args()

    if os.path.isfile(os.path.abspath(options.input)):
        cells_physicell = loadmat(os.path.abspath(options.input))
        data = pd.DataFrame(cells_physicell['cells'])
        data = data.transpose()
        data.to_csv("./output/%s.csv" %os.path.splitext(os.path.basename(os.path.abspath(options.input)))[0])
    else:
        pass