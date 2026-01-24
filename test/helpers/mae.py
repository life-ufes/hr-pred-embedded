import argparse
import pandas as pd
import matplotlib.pyplot as plt


def main(args):
	df = pd.read_csv(args.file)

	df["abs_error"] = abs(df["predicted_values"] - df["ground_truth_values"])
	df["cumulative_mae"] = df["abs_error"].expanding().mean()
	df["moving_mae"] = df["abs_error"].rolling(window=100).mean()

	plt.figure(figsize=(12, 6))

	plt.plot(df.index, df['abs_error'], color='gray', linestyle=':', alpha=0.4, label='Erro Absoluto (Ponto a Ponto)')
	plt.plot(df.index, df['cumulative_mae'], color='blue', linewidth=1, label='MAE Acumulado (Média Total)')
	plt.plot(df.index, df['moving_mae'], color='red', linewidth=1, label='MAE Móvel (Tendência Atual)')

	plt.title('Análise Comparativa de Erro ao Longo do Tempo')
	plt.xlabel('Amostras / Tempo')
	plt.ylabel('Erro (BPM)')
	plt.legend()
	plt.grid(True, which='both', linestyle='--', alpha=0.5)

	plt.tight_layout()
	plt.savefig(f"{args.output}/{args.label}.png", dpi=300)


if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument("-f", "--file", type=str, required=True, help='Data file name')
	parser.add_argument("-l", "--label", type=str, required=False, help='Label for the plot', default="mae")
	parser.add_argument("-o", "--output", type=str, required=False, help='Output file name', default="selected_output")

	args = parser.parse_args()
	main(args=args)
