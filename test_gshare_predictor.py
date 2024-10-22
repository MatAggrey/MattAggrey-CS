from gshare_predictor import GSharePredictor

# Initialize the GShare predictor
predictor = GSharePredictor()

# List of simulated branches
# Each tuple represents (Program Counter, Actual Outcome, Target Address)
# Outcome: 1 means TAKEN, 0 means NOT-TAKEN
branches = [
    (0b1100, 1, 0b10100),  # Branch taken, target address 20
    (0b0011, 0, None),      # Branch not taken
    (0b1010, 1, 0b11110),   # Branch taken, target address 30
    (0b0101, 0, None),      # Branch not taken
    (0b1100, 1, 0b10100),   # Branch taken again, target should be 20
    (0b1100, 0, None),      # Branch not taken this time
]

print("Branch Prediction Simulation:\n")

for pc, actual_outcome, target in branches:
    prediction, predicted_target = predictor.predict(pc)
    
    if prediction:
        if predicted_target == target:
            print(f"PC: {pc:04b} - Prediction: TAKEN to {predicted_target:04b} - Correct")
        else:
            print(f"PC: {pc:04b} - Prediction: TAKEN to {predicted_target:04b} - Incorrect, should be {target:04b}")
    else:
        if not actual_outcome:
            print(f"PC: {pc:04b} - Prediction: NOT-TAKEN - Correct")
        else:
            print(f"PC: {pc:04b} - Prediction: NOT-TAKEN - Incorrect, should be TAKEN to {target:04b}")
    
    # Update the predictor with the actual outcome and target address
    predictor.update(pc, actual_outcome, target)
