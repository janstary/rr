struct inst {
	char	i;
	int	r;
};

struct rm {
	unsigned	numi;
	unsigned	numr;
	struct inst	inst[1024];
	unsigned	regs[1024];
};
