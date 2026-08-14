
import re
import json
import os

class PersianTehraniNormalizer:
    def __init__(self, rules_dir=os.path.join(os.path.dirname(__file__), "rules")):
        self.rules_dir = rules_dir
        self.vocab_map = self._load_json("vocab_map.json")
        self.verb_patterns = self._load_json("verb_patterns.json")
        
    def _load_json(self, filename):
        path = os.path.join(self.rules_dir, filename)
        if os.path.exists(path):
            with open(path, 'r', encoding='utf-8') as f:
                return json.load(f)
        return {}
    
    def normalize(self, text: str) -> str:
        """Normalize Persian text to Tehrani colloquial dialect."""
        # 1. Vocabulary replacements (exact matches, longest first)
        for formal, tehrani in sorted(self.vocab_map.items(), key=lambda x: -len(x[0])):
            text = text.replace(formal, tehrani)
        
        # 2. Verb pattern replacements (regex)
        for pattern_dict in self.verb_patterns:
            pattern = pattern_dict["pattern"]
            replacement = pattern_dict["replacement"]
            text = re.sub(pattern, replacement, text)
        
        return text

if __name__ == "__main__":
    normalizer = PersianTehraniNormalizer()
    test_text = "شما کتاب خود را دارید. می‌روم می‌خواهم می‌کنم."
    result = normalizer.normalize(test_text)
    print(f"Input: {test_text}")
    print(f"Output: {result}")
