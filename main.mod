main {
    var nOffices = new Array(1);
    nOffices[0] = 16;
    //nOffices[0] = 16; nOffices[1] = 32; nOffices[2] = 64; nOffices[3] = 128;
    //nOffices[4] = 256; nOffices[5] = 512; nOffices[6] = 1024; nOffices[7] = 2048;
    //nOffices[8] = 4096; nOffices[9] = 8192; nOffices[10] = 16384; nOffices[11] = 32768;
    //nOffices[3] = 65536;
    var nCenters = new Array(12);
    nCenters[0] = 16; nCenters[1] = 32; nCenters[2] = 64; nCenters[3] = 128;
    nCenters[4] = 256; nCenters[5] = 512; nCenters[6] = 1024; nCenters[7] = 2048;
    nCenters[8] = 4096; nCenters[9] = 8192; nCenters[10] = 16384; nCenters[11] = 32768;     
    var nSegments = new Array(1);
    nSegments[0] = 4;
    var datPath = "set2/";
    var ofile = new IloOplOutputFile(datPath + "last_execution.out", true);
    for (var o = 0; o < nOffices.length; ++o) {
        for (var c = 0; c < nCenters.length; ++c) {
            for (var s = 0; s < nSegments.length; ++s) {
                var src = new IloOplModelSource("L1.mod");
                var def = new IloOplModelDefinition(src);
                var cplex = new IloCplex();
                var model = new IloOplModel(def, cplex);
                var datFile = datPath + nOffices[o] + "_" + nCenters[c] + "_" + nSegments[s] + ".dat";
                writeln("----------------------------------------------------------------------------");
                writeln("  " + datFile);
                writeln("----------------------------------------------------------------------------");
                var data = new IloOplDataSource(datFile);
                model.addDataSource(data);
                model.generate();
                cplex.epgap = 0.01;
                cplex.tilim = 2 * 60;
                var strTime = new Date().getTime();
                if (cplex.solve()) {
                    var endTime = new Date().getTime();
                    ofile.writeln(nOffices[o] + "\t" + nCenters[c] + "\t" + nSegments[s] + "\t" + cplex.getObjValue() + "\t" + ( endTime - strTime ));
                } else {
                    var endTime = new Date().getTime();
                    ofile.writeln(nOffices[o] + "\t" + nCenters[c] + "\t" + nSegments[s] + "\t" + "NO_SOL" + "\t" + ( endTime - strTime ));
                    break;
                }
                model.end();
                data.end();
                def.end();
                cplex.end();
                src.end();
            }
        }
    }
    ofile.close();
};
/*
  ofile.writeln("Data:");
  for(var i in thisOplModel.r){
     ofile.writeln("y["+i+"]="+thisOplModel.y[i]);
  }
  ofile.writeln("Optimal objective value="+cplex.getObjValue());
  ofile.writeln("Optimal variable values:");
  for(i in thisOplModel.r){
     ofile.writeln("x["+i+"]="+thisOplModel.x[i]);
  }
  */
