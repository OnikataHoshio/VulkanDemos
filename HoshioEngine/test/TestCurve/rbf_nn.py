import torch
import torch.nn as nn
import numpy as np
import matplotlib.pyplot as plt

class RBFLayer(nn.Module):
    def __init__(self, in_features : int, out_features : int):
        super().__init__()
        self.linear = nn.Linear(in_features, out_features, bias = True)
        nn.init.xavier_uniform_(self.linear.weight)
        nn.init.zeros_(self.linear.bias)

    def forward(self, x : torch.Tensor) -> torch.Tensor:
        z = self.linear(x)
        return torch.exp(-(z ** 2) / 2.0)
    

def train(X, Y, num_middle, epochs, X_p):
    print("Training parameters:")
    print(f"X: {X}")
    print(f"Y: {Y}")
    print(f"num_middle: {num_middle}")
    print(f"epochs: {epochs}")

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    model = nn.Sequential(
        RBFLayer(in_features=1, out_features=num_middle),
        nn.Linear(num_middle, 1)
    ).to(device)

    criterion = nn.MSELoss()
    optimizer = torch.optim.Adam(model.parameters())

    X_arr = np.array(X, dtype=np.float32)
    Y_arr = np.array(Y, dtype=np.float32)

    max_x = float(np.max(X_arr))
    max_y = float(np.max(Y_arr))

    X_norm = (X_arr/max_x).reshape(-1, 1)
    Y_norm = (Y_arr/max_y).reshape(-1, 1)

    X_tensor = torch.from_numpy(X_norm).to(device)
    Y_tensor = torch.from_numpy(Y_norm).to(device)

    model.train()

    for _ in range(epochs):
        optimizer.zero_grad()
        pred = model(X_tensor)
        loss = criterion(pred, Y_tensor)
        loss.backward()
        optimizer.step()

    X_p_arr = np.array(X_p, dtype = np.float32).reshape(-1, 1)
    X_p_norm = X_p_arr/max_x
    X_p_tensor = torch.from_numpy(X_p_norm).to(device)

    model.eval()
    with torch.no_grad():
        Y_p_norm = model(X_p_tensor).cpu().numpy().reshape(-1)

    Y_p = (Y_p_norm * max_y).tolist()
    print(f"X_p: {X_p}")
    print(f"Y_p: {Y_p}")
    return Y_p

if __name__ == "__main__":
    X = [-100, -50, 0.0, 50, 100]
    Y = [100, -100, -100, 100, 100]

    X_p = [i for i in range(-100, 101)]

    num_middle = 3

    epochs = 2000

    Y_p = train(X, Y, num_middle, epochs, X_p)

    plt.plot(X, Y, label="train", linewidth=2)
    plt.plot(X_p, Y_p, label="pred", linewidth=2)
    plt.legend()
    plt.show()
    




        