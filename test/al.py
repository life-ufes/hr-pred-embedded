import pandas as pd
import argparse
import matplotlib.pyplot as plt


def main(args):
	df = pd.read_csv(args.file)
	plt.figure(figsize=(12,6))

	plt.plot(df.index, df["Al"], color='blue', linewidth=1, alpha=1, label='Computed Activity Level')

	plt.title('Análise do AL ao Longo do Tempo')
	plt.xlabel('Amostras')
	plt.ylabel('Valor')
	plt.legend()
	plt.grid(True, which='both', linestyle='--', alpha=0.5)

	plt.tight_layout()
	plt.savefig(f"{args.output}/{args.label}.png", dpi=300)

if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument("-f", "--file", type=str, required=True, help='Data file name')
	parser.add_argument("-l", "--label", type=str, required=False, help='Label for the plot', default="AL")
	parser.add_argument("-o", "--output", type=str, required=False, help='Output file name', default="selected_output")
	
	args = parser.parse_args()
	main(args=args)
