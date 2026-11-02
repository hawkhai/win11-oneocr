package com.example.oneocr;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

/** Copy the independent models to an app-private directory before opening. */
public final class OneOcr implements AutoCloseable {
    static { System.loadLibrary("oneocr_jni"); }
    private long handle;
    public OneOcr(String models) { handle = create(models.getBytes(StandardCharsets.UTF_8)); }
    private void requireOpen() { if (handle == 0) throw new IllegalStateException("OCR is closed"); }
    public synchronized String recognize(String path, int maxLines, int resizeWidth, int resizeHeight) {
        requireOpen();
        return new String(file(handle, path.getBytes(StandardCharsets.UTF_8), maxLines, resizeWidth, resizeHeight), StandardCharsets.UTF_8);
    }
    /** BGRA, not Android Bitmap RGBA. Reads from buffer base; stride is bytes. */
    public synchronized String recognizeBgra(ByteBuffer pixels, int width, int height, long stride, int maxLines) {
        requireOpen();
        if (!pixels.isDirect()) throw new IllegalArgumentException("A direct BGRA ByteBuffer is required");
        return new String(bgra(handle, pixels, width, height, stride, maxLines), StandardCharsets.UTF_8);
    }
    @Override public synchronized void close() { if (handle != 0) { destroy(handle); handle = 0; } }
    private static native long create(byte[] models);
    private static native byte[] file(long handle, byte[] path, int maxLines, int resizeWidth, int resizeHeight);
    private static native byte[] bgra(long handle, ByteBuffer pixels, int width, int height, long stride, int maxLines);
    private static native void destroy(long handle);
}
