// Export selected algorithm functions without repeating whole-project analysis.
// @category OneOCR
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;

public class ExportFunctions extends GhidraScript {
    public void run() throws Exception {
        String[] args=getScriptArgs();
        Path output=Paths.get(args[0]);Files.createDirectories(output);
        DecompInterface decompiler=new DecompInterface();decompiler.openProgram(currentProgram);
        StringBuilder callers=new StringBuilder();
        for(int i=1;i<args.length;i++) {
            var references=currentProgram.getReferenceManager().getReferencesTo(toAddr(args[i]));
            while(references.hasNext()) {
                var reference=references.next();var caller=getFunctionContaining(reference.getFromAddress());
                callers.append(args[i]).append('\t').append(reference.getFromAddress()).append('\t').append(caller==null?"data":caller.getEntryPoint()).append('\n');
            }
        }
        Files.write(output.resolve("callers.tsv"),callers.toString().getBytes(StandardCharsets.UTF_8));
        ArrayDeque<Function> queue=new ArrayDeque<>();HashMap<Function,Integer> depths=new HashMap<>();
        for(int i=1;i<args.length;i++) {
            Function function=getFunctionContaining(toAddr(args[i]));
            if(function==null) {println("Missing function: "+args[i]);continue;}
            queue.add(function);depths.put(function,0);
        }
        HashSet<Function> visited=new HashSet<>();
        while(!queue.isEmpty() && visited.size()<120 && !monitor.isCancelled()) {
            Function function=queue.remove();if(function.isExternal() || !visited.add(function))continue;
            DecompileResults result=decompiler.decompileFunction(function,90,monitor);
            if(result.decompileCompleted()) {
                String text="// "+function.getEntryPoint()+" "+function.getName()+"\n"+result.getDecompiledFunction().getC();
                Files.write(output.resolve(function.getEntryPoint()+".c"),text.getBytes(StandardCharsets.UTF_8));
            }
            if(depths.get(function)==0) {
                StringBuilder instructions=new StringBuilder();
                var iterator=currentProgram.getListing().getInstructions(function.getBody(),true);
                while(iterator.hasNext()) {
                    var instruction=iterator.next();
                    instructions.append(instruction.getAddress()).append('\t').append(instruction).append('\n');
                }
                Files.write(output.resolve(function.getEntryPoint()+".asm"),instructions.toString().getBytes(StandardCharsets.UTF_8));
            }
            if(depths.get(function)<1)for(Function called:function.getCalledFunctions(monitor)) {
                if(!depths.containsKey(called)){queue.add(called);depths.put(called,depths.get(function)+1);}
            }
        }
        StringBuilder constants=new StringBuilder();
        for(long address=0x182576360L;address<0x1825765c0L;address+=4)
            constants.append(Long.toHexString(address)).append('\t').append(getFloat(toAddr(address))).append('\n');
        Files.write(output.resolve("detector-constants.tsv"),constants.toString().getBytes(StandardCharsets.UTF_8));
        decompiler.dispose();println("Exported "+visited.size()+" selected functions");
    }
}
