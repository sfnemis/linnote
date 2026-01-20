# Calculator

LinNote's calculator evaluates expressions in real-time.

**Activate:**
```
calc
```

## Basic Usage

Type any expression. Add `=` at the end to see the result:

```
2 + 2 =
// Result: 4

100 / 4 =
// Result: 25
```

## Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `5 + 3 = 8` |
| `-` | Subtraction | `10 - 4 = 6` |
| `*` | Multiplication | `6 * 7 = 42` |
| `/` | Division | `15 / 3 = 5` |
| `^` | Power | `2^8 = 256` |
| `%` | Modulo | `17 % 5 = 2` |
| `()` | Parentheses | `(2 + 3) * 4 = 20` |

## Percentages

```
100 + 18% =
// Result: 118

500 - 20% =
// Result: 400

what is 15% of 200 =
// Result: 30
```

## Variables

Store values for reuse:

```
price = 100
quantity = 5
discount = 10

subtotal = price * quantity =
// Result: 500

discount_amount = subtotal * discount / 100 =
// Result: 50

total = subtotal - discount_amount =
// Result: 450
```

## Functions

| Function | Description | Example |
|----------|-------------|---------|
| `sqrt(x)` | Square root | `sqrt(144) = 12` |
| `abs(x)` | Absolute value | `abs(-5) = 5` |
| `sin(x)` | Sine (radians) | `sin(0) = 0` |
| `cos(x)` | Cosine | `cos(0) = 1` |
| `tan(x)` | Tangent | `tan(0) = 0` |
| `log(x)` | Natural log | `log(e) = 1` |
| `log10(x)` | Log base 10 | `log10(100) = 2` |
| `floor(x)` | Round down | `floor(3.7) = 3` |
| `ceil(x)` | Round up | `ceil(3.2) = 4` |
| `round(x)` | Round nearest | `round(3.5) = 4` |
| `min(a,b)` | Minimum | `min(5, 3) = 3` |
| `max(a,b)` | Maximum | `max(5, 3) = 5` |

## Constants

| Constant | Value |
|----------|-------|
| `pi` | 3.14159265359 |
| `e` | 2.71828182846 |

---

## Text Analysis Keywords

Analyze numbers in your note:

### sum

Adds all numbers:

```
Groceries:
Milk 4.50
Bread 2.30
Eggs 6.00

sum
```
**Result:** `Sum: 12.80`

### avg

Calculates average:

```
Test Scores:
85
92
78
88

avg
```
**Result:** `Average: 85.75`

### count

Counts numbers:

```
Items in list:
5 apples
3 bananas
8 oranges

count
```
**Result:** `Count: 3`

---

## Practical Examples

### Tip Calculator

```
bill = 85.50
tip_percent = 18
people = 4

tip = bill * tip_percent / 100 =
// Result: 15.39

total = bill + tip =
// Result: 100.89

per_person = total / people =
// Result: 25.22
```

### Area Calculation

```
radius = 5
area = pi * radius^2 =
// Result: 78.54
```
