import argparse
import pandas as pd
import matplotlib.pyplot as plt


def main(args):
	df = pd.read_csv(args.file)

	plt.figure(figsize=(12,6))

	x = df.index + 60

	plt.plot(x, df['hr_reg'], color='blue', linewidth=1, linestyle='--', alpha=0.5, label='HR_reg')
	plt.plot(x, df['predicted_values'], color="green", linewidth=1, label="HR_pred")
	plt.plot(x, df['ground_truth_values'], color="red", linewidth=1, label="HR_gt")
	
	plt.axvline(x=60, color='yellow', linestyle=':', linewidth=1, label='Train')
	plt.axvline(x=420, color='blue', linestyle=':', linewidth=1, label='Test')

	plt.title('Análise Comparativa dos HRs ao Longo do Tempo')
	plt.xlabel('Amostras')
	plt.ylabel('BPM')
	plt.legend()
	plt.grid(True, which='both', linestyle='--', alpha=0.5)

	plt.tight_layout()
	plt.savefig(f"{args.output}/{args.label}.png", dpi=300)


if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument("-f", "--file", type=str, required=True, help='Data file name')
	parser.add_argument("-l", "--label", type=str, required=False, help='Label for the plot', default="hr")
	parser.add_argument("-o", "--output", type=str, required=False, help='Output file name', default="selected_output")
	
	args = parser.parse_args()
	main(args=args)
