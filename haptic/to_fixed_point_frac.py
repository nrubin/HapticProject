#let's convert floats to fixed point fractions!

f = 1/250.0
fp = "0b"
msb = 0.5
lsb = 1.0/(2**16)
current_bit = msb

for index in range(1,17):
	current_bit = 1.0/(2**index)
	print "f = %s, current_bit = %s" % (f,current_bit)
	diff = f - current_bit
	if diff >= 0.0:
		f = f - current_bit
		fp = fp + "1"
	else:
		fp = fp + "0"

print fp
