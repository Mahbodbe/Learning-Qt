
import sys
sys.path.insert(0, "/root/.hermes/skills/persian-tehrani-normalizer")
from normalizer import PersianTehraniNormalizer

normalizer = PersianTehraniNormalizer()

test_cases = [
    "شما کتاب خود را دارید",
    "من می‌روم می‌خواهم می‌کنم",
    "شما می‌خواهید می‌خواهند می‌خواهیم",
    "خانه شما زیباست",
    "اکنون می‌روم",
    "خانه شما زیباست",
    "اکنون می‌روم",
    "چه خبر",
    "خوب هستید",
    "مرسی",
]

print("Testing Persian Tehrani Normalizer:")
print("=" * 50)
for test in test_cases:
    result = normalizer.normalize(test)
    print(f"Input:  {test}")
    print(f"Output: {result}")
    print("-" * 40)
