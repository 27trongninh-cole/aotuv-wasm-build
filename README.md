# aoTuV → WASM build kit

Mục tiêu: build `aotuv.wasm` + `aotuv.js` từ mã nguồn aoTuV beta6.03
(https://github.com/AO-Yumi/vorbis_aotuv), để dùng đúng bộ mã hoá khớp với
`packed_codebooks_aoTuV_603.bin` bạn đang có trong tool web.

**Đọc NOTES.md trước khi bắt đầu** — có vài điểm rủi ro cần biết trước.

## Yêu cầu

- Một máy tính (Windows/Mac/Linux đều được) có cài **Docker Desktop**.
  (Không cần cài Emscripten thủ công — Docker lo hết.)
- Kết nối mạng (để tải source libogg + aoTuV lúc build).
- ~30-60 phút, chủ yếu là thời gian chờ build.

## Các bước

### 1. Tải repo này về máy

Giải nén toàn bộ thư mục `aotuv-wasm-build/` vào một chỗ, ví dụ
`C:\aotuv-wasm-build` hoặc `~/aotuv-wasm-build`.

### 2. Mở terminal / Command Prompt tại thư mục đó

```bash
cd aotuv-wasm-build
```

### 3. Build Docker image

```bash
docker build -t aotuv-builder .
```

Lệnh này tải image nền `emscripten/emsdk` (khoảng 1-2GB, chỉ tải 1 lần) rồi
cài các công cụ cần thiết (autoconf, git, v.v).

### 4. Chạy container để thực hiện build thật sự

```bash
docker run --rm -v "$(pwd)/output:/build/output" aotuv-builder
```

Trên Windows PowerShell, thay `$(pwd)` bằng `${PWD}`:

```powershell
docker run --rm -v "${PWD}/output:/build/output" aotuv-builder
```

Script `scripts/build_all.sh` sẽ tự động:
1. Tải source `libogg`
2. Tải source `vorbis_aotuv` (aoTuV beta6.03)
3. Build `libogg` → `.a`
4. Build `libvorbis` (bản aoTuV) → `.a`
5. Compile `src/aotuv_wrapper.c` cùng các thư viện trên → `aotuv.wasm` + `aotuv.js`

Khi chạy xong, bạn sẽ thấy trong thư mục `output/`:
```
output/
  aotuv.js
  aotuv.wasm
```

### 5. Kiểm tra nhanh

Mở `output/` — nếu thấy 2 file trên với kích thước > 0 byte, bước build coi
như thành công về mặt compile. (Chưa chắc encode ra đúng kết quả — xem bước 6.)

### 6. Test thử trước khi tích hợp vào tool chính

Trước khi gắn vào tool web thật, nên test độc lập:
1. Tạo 1 file HTML đơn giản, include `aotuv.js`, load 1 file .wav mẫu
2. Gọi hàm theo `js/example_usage.js` (đã có sẵn code mẫu + comment)
3. Tải file `.ogg` ra, thử đưa qua bước match-codebook hiện có trong tool
   → nếu khớp codebook thành công, nghĩa là build đúng.

### 7. Copy vào tool web

Copy `aotuv.js` + `aotuv.wasm` vào cùng thư mục host tool
(`wav_to_wem-2.html`), thêm:

```html
<script src="aotuv.js"></script>
```

vào file HTML, rồi thay đoạn code hiện tại đang gọi encoder libvorbis chuẩn
bằng lời gọi `encodeWavToOggWithAotuv(...)` (xem `js/example_usage.js`).

## Nếu build lỗi

Lỗi thường gặp nhất là ở bước 4 (`emconfigure ./configure` cho
`vorbis_aotuv`) vì repo đó có thể không có sẵn cấu trúc autotools đầy đủ
(thiếu `configure`, thiếu `Makefile.am`...). Xem **NOTES.md mục "Nếu
autoconf thất bại"** để biết hướng xử lý thủ công.

Gửi lại nguyên văn log lỗi (toàn bộ output terminal) nếu cần mình hỗ trợ đọc lỗi tiếp.
