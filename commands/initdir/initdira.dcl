
	declare
		seldsk	entry	(fixed(7)) returns(ptr),
		settrk	entry	(fixed(15)),
		setsec	entry	(fixed(15)),
		rdsec	entry	returns(fixed(7)),
		wrsec	entry	(fixed(7)) returns(fixed(7)),
		sectrn	entry	(fixed(15), ptr) returns(fixed(15)),
		bstdma	entry	(ptr);

	declare		/* CCPM special functions   whf 1/14/82 */
		openvec   entry returns(fixed(15)),
		syslock   entry	returns(fixed(7)),
		sysunlock entry,
		conlock   entry returns(fixed(7)),
		conunlock entry;
