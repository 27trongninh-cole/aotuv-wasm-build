/*
 * example_usage.js
 *
 * Ví dụ gọi aotuv.wasm (sau khi build) từ trong trình duyệt / tool web
 * của bạn. Copy 2 file aotuv.js + aotuv.wasm (output của build) vào cùng
 * thư mục với tool, rồi include như dưới đây.
 *
 * <script src="aotuv.js"></script>   <-- thêm vào HTML trước script chính
 */

async function encodeWavToOggWithAotuv(pcmInt16Interleaved, numSamplesPerChannel, channels, sampleRate, mode, quality, nominalBitrate) {
  // AotuvModule được định nghĩa bởi aotuv.js (MODULARIZE=1, EXPORT_NAME=AotuvModule)
  const Module = await AotuvModule();

  // 1. Cấp phát vùng nhớ trong WASM heap và copy PCM vào
  const bytesPerSample = 2; // int16
  const pcmByteLength = pcmInt16Interleaved.length * bytesPerSample;
  const pcmPtr = Module._malloc(pcmByteLength);
  Module.HEAPU8.set(new Uint8Array(pcmInt16Interleaved.buffer), pcmPtr);

  // 2. Cấp phát 4 byte để nhận out_len (int*) qua tham số output
  const outLenPtr = Module._malloc(4);

  // 3. Gọi hàm C. mode=0 -> dùng "quality" (VBR); mode=1 -> dùng "nominalBitrate" (ABR/managed).
  //    Tham số không dùng tới (quality khi mode=1, hoặc nominalBitrate khi mode=0) truyền giá trị
  //    bất kỳ cũng được, phía C sẽ bỏ qua.
  const oggPtr = Module.ccall(
    "aotuv_encode_wav_to_ogg",
    "number",           // return type: con trỏ (number trong JS)
    ["number", "number", "number", "number", "number", "number", "number", "number"],
    [pcmPtr, numSamplesPerChannel, channels, sampleRate, mode, quality, nominalBitrate, outLenPtr]
  );

  if (oggPtr === 0) {
    Module._free(pcmPtr);
    Module._free(outLenPtr);
    throw new Error("aoTuV encode thất bại (encoder trả về NULL)");
  }

  // 4. Đọc out_len ra từ WASM heap
  const outLen = Module.HEAPU8[outLenPtr] |
                 (Module.HEAPU8[outLenPtr + 1] << 8) |
                 (Module.HEAPU8[outLenPtr + 2] << 16) |
                 (Module.HEAPU8[outLenPtr + 3] << 24);

  // 5. Copy buffer .ogg ra khỏi WASM heap thành Uint8Array JS thường
  const oggBytes = new Uint8Array(Module.HEAPU8.buffer, oggPtr, outLen).slice();

  // 6. QUAN TRỌNG: free() cả 3 vùng nhớ đã cấp phát trong WASM, tránh leak
  Module._free(pcmPtr);
  Module._free(outLenPtr);
  Module._free(oggPtr);

  return oggBytes; // đây chính là nội dung file .ogg, đưa tiếp vào bước OGG->WEM hiện có
}

/*
 * Cách lấy pcmInt16Interleaved từ 1 file .wav đã có trong tool (Web Audio API):
 *
 *   const arrayBuffer = await wavFile.arrayBuffer();
 *   const audioCtx = new OfflineAudioContext(2, 1, 48000); // tạm, chỉ để decode
 *   const audioBuffer = await audioCtx.decodeAudioData(arrayBuffer);
 *   const channels = audioBuffer.numberOfChannels;
 *   const sampleRate = audioBuffer.sampleRate;
 *   const numSamples = audioBuffer.length;
 *   const pcm = new Int16Array(numSamples * channels);
 *   for (let ch = 0; ch < channels; ch++) {
 *     const chData = audioBuffer.getChannelData(ch); // Float32Array, -1..1
 *     for (let i = 0; i < numSamples; i++) {
 *       let s = Math.max(-1, Math.min(1, chData[i]));
 *       pcm[i * channels + ch] = s < 0 ? s * 32768 : s * 32767;
 *     }
 *   }
 *   const ogg = await encodeWavToOggWithAotuv(pcm, numSamples, channels, sampleRate, 0.4);
 *   // "ogg" giờ đây là file .ogg encode bằng đúng aoTuV 6.03, đưa vào bước
 *   // match-codebook + đóng gói .wem hiện có trong tool của bạn.
 */
