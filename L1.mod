/*********************************************
 * OPL 12.4 Model
 * Author: oliveras
 * Creation Date: Dec 21, 2012 at 11:55:35 AM
 *********************************************/

 int nOffices = ...;
 int nBackupCenters = ...;
 int nSegments = ...;
 
 range O = 1..nOffices;
 range C = 1..nBackupCenters;
 range P = 1..nSegments;
 

 float M = 10000;
 
 // Demand for each office (in PB)
 int demand[o in O] = ...; 
 // Whether office o is conected with each center c
 int united[o in O][c in C] = ...;
 
 // Capacity of each backup center (in PB)
 int capacity[c in C] = ...;

 // Fixed cost of using each backup center (in thousands of euros)
 int fixedCost[c in C] = ...;
 // Cost of one PB in each segment
 int costPerPB[p in P] = ...;
 // Minimum number of PB to apply the segment price
 int minimumPB[p in P] = ...;
  
 dvar int+ x[o in O][c in C][p in P];	// Number of PBs of office o stored in center c at price p
 dvar boolean used[c in C];				// Whether center c is used
 dvar boolean inSegment[c in C][p in P];// Whether consume in center c allows apply the segment price p
 

 minimize sum(c in C) ( fixedCost[c] * used[c] ) + sum(c in C, o in O, p in P) ( x[o][c][p] * costPerPB[p] );
 
 subject to {
 	
 	// At least each office can store two times its demeand
 	forall (o in O)
 	    sum(c in C, p in P) x[o][c][p] >= 2 * demand[o];
 	;
 	
 	// At most demand[o] in one center for each office -> At least the data of one office will be distributed in two centers
 	forall (o in O, c in C)
 	    sum(p in P) x[o][c][p] <= demand[o]*united[o][c];
 	;
 	    
 	// No exceed center capacity
 	forall (c in C)
 	    sum(o in O, p in P) x[o][c][p] <= capacity[c];
 	;
 	
 	// For each center, set if it is used
 	forall (c in C) {
 	  	//used[c] == (( sum(o in O, p in P) x[o][c][p] ) != 0);
 	  	used[c]*M >= ( sum(o in O, p in P) x[o][c][p] );
 	  	used[c] <= ( sum(o in O, p in P) x[o][c][p] );
	};
 	
 	// For each center and segment price, the consume is at least the minimum number of PB needed to apply the segment price
 	forall (c in C, p in P)
 	    sum(o in O) x[o][c][p] >= minimumPB[p] * inSegment[c][p];
 	;

	// For each center, only one one price segment can be active
 	forall (c in C)
 	  	sum(p in P) ( inSegment[c][p] ) <= 1;
 	;
 	
 	// For each center, set if one segment is used
 	forall (c in C, p in P) {
 	  	//inSegment[c][p] == (( sum(o in O) x[o][c][p] ) != 0);
 	  	inSegment[c][p]*M >= ( sum(o in O) x[o][c][p] );
 	  	inSegment[c][p] <= ( sum(o in O) x[o][c][p] );
   };
}

execute {
  
  writeln("Centres being used:");
  for (var c in C)
  	if (used[c] == 1) {
        var cost = 0;
        for(var p in P) {
        	cost += inSegment[c][p] * costPerPB[p];        
        }
  		writeln(c + " (with capacity " + capacity[c] + ", fixed cost " + fixedCost[c] + " and price " + cost + ")");
  		for (var o in O) {
			var amount = 0;
			for(var p in P) {
		    	amount += x[o][c][p];
		    }
    		if (amount > 0) writeln(" " + amount + " PB to office " + o); 
    	}
    	writeln();  		  
  }  		
  
  writeln("Information about offices:");
  for (o in O) {
  	writeln("Office " + o + " (with demand " + demand[o] + ")");
  	for (c in C) {
		var amount = 0;
		for(var p in P) {
	    	amount += x[o][c][p];
	    }
		if (amount > 0) writeln(amount + " from center " + c);  			  
    }  	    
    writeln();
  }    
  	
}   