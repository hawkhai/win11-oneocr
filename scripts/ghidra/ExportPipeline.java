// Export source-recovery evidence; runs inside Ghidra 11.3.2 headless.
// @category OneOCR
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Symbol;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;

public class ExportPipeline extends GhidraScript {
    public void run() throws Exception {
        String[] args = getScriptArgs();
        Path output = Paths.get(args[0]);
        Files.createDirectories(output);
        DecompInterface decompiler = new DecompInterface();
        decompiler.openProgram(currentProgram);
        ArrayDeque<Function> queue = new ArrayDeque<>();
        HashMap<Function,Integer> depth = new HashMap<>();
        StringBuilder vtables = new StringBuilder();
        for (Symbol symbol : currentProgram.getSymbolTable().getSymbolIterator(true)) {
            String full=symbol.getName(true);
            if (!full.contains("OneOCR::") || !full.contains("vftable")) continue;
            vtables.append(full).append('\t').append(symbol.getAddress()).append('\n');
            boolean selected=full.startsWith("OneOCR::") && !full.contains("::Interop::") && !full.contains("Proto") && (full.contains("TextPipeline") || full.contains("TextDetector") || full.contains("TextNormalizer") || full.contains("TextAnalyzer") || full.contains("TextLineRecognizer") || full.contains("ArgMax") || full.contains("CnnDbLstm") || full.contains("AuxMltCls") || full.contains("RelationRCNN") || full.contains("BaseRCNN") || full.contains("Confidence") || full.contains("FeatureExtract"));
            for (int slot=0;slot<30;slot++) {
                long address=getLong(symbol.getAddress().add(slot*8));
                Function f=getFunctionAt(toAddr(address));
                if (f==null) break;
                vtables.append("  ").append(slot).append('\t').append(f.getEntryPoint()).append('\t').append(f.getName()).append('\n');
                if(selected && !depth.containsKey(f)) {queue.add(f);depth.put(f,0);}
            }
        }
        Files.write(output.resolve("vtables.tsv"),vtables.toString().getBytes(StandardCharsets.UTF_8));
        for (String name : new String[]{"RunOcrPipeline","CreateOcrPipeline","GetOcrLineStyle","GetOcrWordConfidence"}) {
            for (Symbol symbol : currentProgram.getSymbolTable().getGlobalSymbols(name)) {
                Function f = getFunctionAt(symbol.getAddress());
                if (f != null) { queue.add(f); depth.put(f,0); }
            }
        }
        for (int i=1;i<args.length;i++) {
            Function f = getFunctionContaining(toAddr(args[i]));
            if (f != null) { queue.add(f); depth.put(f,0); }
        }
        StringBuilder listing = new StringBuilder();
        for (Function f : currentProgram.getFunctionManager().getFunctions(true))
            listing.append(f.getEntryPoint()).append('\t').append(f.getName()).append('\t').append(f.getBody().getNumAddresses()).append('\n');
        Files.write(output.resolve("functions.tsv"),listing.toString().getBytes(StandardCharsets.UTF_8));
        HashSet<Function> visited = new HashSet<>();
        while (!queue.isEmpty() && visited.size()<400 && !monitor.isCancelled()) {
            Function f = queue.remove();
            if (f.isExternal() || !visited.add(f)) continue;
            DecompileResults result = decompiler.decompileFunction(f,60,monitor);
            if (result.decompileCompleted()) {
                String text = "// "+f.getEntryPoint()+" "+f.getName()+"\n"+result.getDecompiledFunction().getC();
                Files.write(output.resolve(f.getEntryPoint()+".c"),text.getBytes(StandardCharsets.UTF_8));
            }
            int d = depth.get(f);
            if (d<4) for (Function called : f.getCalledFunctions(monitor)) {
                if (!depth.containsKey(called)) { depth.put(called,d+1); queue.add(called); }
            }
        }
        decompiler.dispose();
        println("Exported "+visited.size()+" reachable functions to "+output);
    }
}
