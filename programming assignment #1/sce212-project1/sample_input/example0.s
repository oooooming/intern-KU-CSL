	.data
data1:	.word	100
	.text
main:
	la	$5, data1
	la	$6, data1
	add	$10, $5, $6
	sub	$11, $5, $6
