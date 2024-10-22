
#Branch History Table (BHT)
class BHT:
    def __init__(self):
        # 16 entries, 4 bits of history each, initialized to zero
        self.entries = [0b0000 for _ in range(16)]
    
    def get_history(self, index):
        return self.entries[index]
    
    def update_history(self, index, outcome):
        # Shift the history and insert the latest branch outcome (1 for TAKEN, 0 for NOT-TAKEN)
        self.entries[index] = ((self.entries[index] << 1) | outcome) & 0b1111  # Keep 4 bits


#Pattern History Table (PHT):
class PHT:
    def __init__(self):
        # 16 entries, 2 bits each, initialized to weakly NOT-TAKEN (01)
        self.entries = [0b01 for _ in range(16)]
    
    def get_state(self, index):
        return self.entries[index]
    
    def update_state(self, index, taken):
        state = self.entries[index]
        if taken:
            if state < 0b11:
                self.entries[index] += 1  # Move towards strongly TAKEN (11)
        else:
            if state > 0b00:
                self.entries[index] -= 1  # Move towards strongly NOT-TAKEN (00)


class BTBEntry:
    def __init__(self, pc, target):
        self.pc = pc
        self.target = target


#Branch Target Buffer (BTB)
class BTB:
    def __init__(self):
        # Fully associative, 16-entry BTB with FIFO replacement
        self.entries = []
    
    def lookup(self, pc):
        for entry in self.entries:
            if entry.pc == pc:
                return entry.target
        return None
    
    def update(self, pc, target):
        if len(self.entries) >= 16:
            self.entries.pop(0)  # Remove oldest entry (FIFO)
        self.entries.append(BTBEntry(pc, target))


#Branch Predictor (Gshare):

class GSharePredictor:
    def __init__(self):
        self.bht = BHT()
        self.pht = PHT()
        self.btb = BTB()
    
    def predict(self, pc):
        # Use the last 4 bits of PC and XOR with BHT history to get PHT index
        bht_index = pc & 0b1111  # Last 4 bits of PC
        history = self.bht.get_history(bht_index)
        pht_index = bht_index ^ history  # XOR operation
        
        # Get prediction from the PHT
        pht_state = self.pht.get_state(pht_index)
        prediction = pht_state >= 0b10  # TAKEN if in state 10 or 11
        
        # Check BTB if predicted TAKEN
        if prediction:
            target = self.btb.lookup(pc)
            if target is not None:
                return prediction, target
        # If not in BTB or NOT-TAKEN, return PC+1 as the next address
        return prediction, pc + 1
    
    def update(self, pc, outcome, target=None):
        # Update PHT based on actual outcome
        bht_index = pc & 0b1111
        history = self.bht.get_history(bht_index)
        pht_index = bht_index ^ history
        self.pht.update_state(pht_index, outcome)
        
        # Update BHT with the latest outcome
        self.bht.update_history(bht_index, outcome)
        
        # If branch was TAKEN, update BTB with the target address
        if outcome and target:
            self.btb.update(pc, target)
