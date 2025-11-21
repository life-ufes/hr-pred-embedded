import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

from app.globals import (
    w0, w1, w2, w3, w4,
    b_low, b_high,
    Al, tau, hr_reg,
    predicted_values, ground_truth_values
)

# Plot setup (global figure)
fig, ax = plt.subplots()
ax.set_title("Heart Rate: Prediction vs Ground Truth")
ax.set_xlabel("Seconds")
ax.set_ylabel("BPM")

line_w0, = ax.plot([], [], label="W0")
line_w1, = ax.plot([], [], label="W1")
line_w2, = ax.plot([], [], label="W2")
line_w3, = ax.plot([], [], label="W3")
line_w4, = ax.plot([], [], label="W4")
line_b_high, = ax.plot([], [], label="B_HIGH")
line_b_low, = ax.plot([], [], label="B_LOW")
line_al, = ax.plot([], [], label="AL")
line_tau, = ax.plot([], [], label="TAU")
line_hr_reg, = ax.plot([], [], label="HR_reg")
line_pred, = ax.plot([], [], label="HR_next")
line_gt, = ax.plot([], [], label="HR_gt")
ax.legend()


def update_plot(frame):
    line_w0.set_data(range(len(w0)), w0)
    line_w1.set_data(range(len(w1)), w1)
    line_w2.set_data(range(len(w2)), w2)
    line_w3.set_data(range(len(w3)), w3)
    line_w4.set_data(range(len(w4)), w4)
    line_b_high.set_data(range(len(b_high)), b_high)
    line_b_low.set_data(range(len(b_low)), b_low)
    line_al.set_data(range(len(Al)), Al)
    line_tau.set_data(range(len(tau)), tau)
    line_hr_reg.set_data(range(len(hr_reg)), hr_reg)
    line_pred.set_data(range(len(predicted_values)), predicted_values)
    line_gt.set_data(range(len(ground_truth_values)), ground_truth_values)

    ax.relim()
    ax.autoscale_view()

    return (
        line_pred, line_gt, line_w0, line_w1, line_w2, line_w3, line_w4, 
        line_b_high, line_b_low, line_al, line_tau, line_hr_reg
    )

def start_realtime_plot():
    ani = FuncAnimation(fig, update_plot, interval=200)
    plt.show()  