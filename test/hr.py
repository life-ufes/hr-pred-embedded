import argparse
import pandas as pd
import matplotlib.pyplot as plt


def main(args):
	df = pd.read_csv(args.file)

	plt.figure(figsize=(12,6))

	plt.plot(df.index, df['hr_reg'], color='blue', linewidth=1, linestyle='--', alpha=0.5, label='HR_reg')
	plt.plot(df.index, df['predicted_values'], color="green", linewidth=1, label="HR_pred")
	plt.plot(df.index, df['ground_truth_values'], color="red", linewidth=1, label="HR_gt")

	plt.title('Análise Comparativa dos HRs ao Longo do Tempo')
	plt.xlabel('Amostras')
	plt.ylabel('BPM')
	plt.legend()
	plt.grid(True, which='both', linestyle='--', alpha=0.5)

	plt.tight_layout()
	plt.savefig("output/hr.png", dpi=300)


if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument("-f", "--file", type=str, required=True, help='Data file name')
	
	args = parser.parse_args()
	main(args=args)
