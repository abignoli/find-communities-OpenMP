int min(int a, int b) {
	return a < b ? a : b;
}

int max(int a, int b) {
	return a > b ? a : b;
}

inline int same(int x, int y){
	return x == y ? 1 : 0;
}

int lower_power_of_2(int n) {
	int result;

	if(n <= 0)
		return -1;

	result = 1;

	while(result * 2 <= n) {
		result *= 2;
	}

	return result;
}

