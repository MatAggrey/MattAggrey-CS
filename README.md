# GShare Branch Predictor

This project implements a GShare branch predictor with a 16-entry Branch History Table (BHT), a 16-entry Pattern History Table (PHT), and a 16-entry Branch Target Buffer (BTB).

## Files

- `gshare_predictor.py`: Contains the implementation of the GShare predictor, including the BHT, PHT, and BTB.
- `test_gshare_predictor.py`: A script to simulate branches and test the predictor's performance.

## How to Run

1. Clone the repository or copy the files into your project folder.
2. Run the test script to simulate branches and view the predictions:
    ```bash
    python test_gshare_predictor.py
    ```

## Sample Output

The test script will output whether the branch predictions were correct or not, based on the actual outcomes.


# Explanations 

## Components Breakdown

### Branch History Table (BHT):
- **Size**: 16 entries
- Each entry contains 4 bits of history, initialized to zero.

### XOR Operation:
- **Inputs**: 4 bits of history from the BHT and the last 4 bits of the Program Counter (PC).
- **Output**: This XOR operation will generate an index for the Pattern History Table (PHT).

### Pattern History Table (PHT):
- **Size**: 16 entries
- Each entry contains a 2-bit state machine initialized to "weakly NOT-TAKEN" (state `01`).
- **States for the state machine**:
  - `00`: Strongly NOT-TAKEN
  - `01`: Weakly NOT-TAKEN
  - `10`: Weakly TAKEN
  - `11`: Strongly TAKEN

### Branch Target Buffer (BTB):
- Fully associative cache with 16 entries.
- **Replacement policy**: FIFO (First-In-First-Out).
- **Tag**: PC of the branch instruction.
- **Data**: Target address of the branch.
- Only branches that are resolved as TAKEN are placed into the BTB.

### Branch Prediction Logic:
- On a branch:
  1. Use the last 4 bits of the PC to XOR with the 4-bit history from the BHT to index into the PHT.
  2. Use the 2-bit state in the PHT to make a prediction (TAKEN or NOT-TAKEN).
  3. If the branch is predicted TAKEN, check if there is a matching entry in the BTB.
  4. If a BTB entry is found, fetch from the stored target address; otherwise, fetch speculatively from `PC+1`.


# Files Strcuture 

/gshare_predictor_project
│
├── gshare_predictor.py           # The GShare predictor implementation
├── test_gshare_predictor.py      # The script to test the branch predictor
└── README.md                     # (Optional) A description of the project and instructions
