// Export direct string/data cross references with decompiled code and assembly.
// @category OneOCR
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;

public class ExportReferences extends GhidraScript {
    public void run() throws Exception {
        String[] args=getScriptArgs(); Path output=Paths.get(args[0]);Files.createDirectories(output);
        DecompInterface decompiler=new DecompInterface();decompiler.openProgram(currentProgram);
        StringBuilder references=new StringBuilder();Set<String> seen=new HashSet<>();
        for(int i=1;i<args.length;i++) {
            var iterator=currentProgram.getReferenceManager().getReferencesTo(toAddr(args[i]));
            while(iterator.hasNext()) {
                var reference=iterator.next();var function=getFunctionContaining(reference.getFromAddress());
                references.append(args[i]).append('\t').append(reference.getFromAddress()).append('\t').append(function==null?"data":function.getEntryPoint()).append('\n');
                if(function==null || !seen.add(function.getEntryPoint().toString()))continue;
                var result=decompiler.decompileFunction(function,120,monitor);
                if(result.decompileCompleted())Files.writeString(output.resolve(function.getEntryPoint()+".c"),result.getDecompiledFunction().getC(),StandardCharsets.UTF_8);
                StringBuilder assembly=new StringBuilder();var instructions=currentProgram.getListing().getInstructions(function.getBody(),true);
                while(instructions.hasNext()){var instruction=instructions.next();assembly.append(instruction.getAddress()).append('\t').append(instruction).append('\n');}
                Files.writeString(output.resolve(function.getEntryPoint()+".asm"),assembly.toString(),StandardCharsets.UTF_8);
            }
        }
        Files.writeString(output.resolve("references.tsv"),references.toString(),StandardCharsets.UTF_8);decompiler.dispose();
    }
}
