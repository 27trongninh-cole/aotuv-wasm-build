# Ghi chú quan trọng — đọc trước khi build

Repo này được soạn dựa trên hiểu biết chung về cách build libvorbis/Emscripten,
**không phải đã chạy thử thành công trên máy thật** (môi trường hiện tại không
có kết nối mạng để tải source và build). Có vài điểm rủi ro cụ thể:

## 1. Cấu trúc repo `AO-Yumi/vorbis_aotuv` có thể không khớp 100%

Mình chưa xem trực tiếp được toàn bộ nội dung repo đó (chỉ thấy nó tồn tại
qua tìm kiếm web). Hai khả năng:

- **Nếu repo chứa toàn bộ mã nguồn libvorbis đã patch sẵn** (có `configure`,
  `Makefile.am`, thư mục `lib/`, `include/`...) → script `build_all.sh` chạy
  thẳng, khả năng cao là ổn.
- **Nếu repo chỉ chứa file patch/diff** (vd `.patch` áp lên libvorbis gốc
  của Xiph.Org) → cần thêm bước: tải libvorbis gốc từ
  `github.com/xiph/vorbis`, rồi `patch -p1 < aotuv.patch` trước khi
  `emconfigure`. Nếu build báo lỗi kiểu "no configure script" hoặc
  "directory not found", đây là nguyên nhân — sửa `build_all.sh` bước 4
  bằng cách:
  ```bash
  git clone --depth 1 --branch v1.3.7 https://github.com/xiph/vorbis.git libvorbis
  cd libvorbis
  patch -p1 < ../vorbis_aotuv/<tên_file_patch>.patch
  ```

## 2. API `vorbis_encode_init_vbr` — tham số quality

Thang đo `quality` trong code C là `-0.1` đến `1.0` (API gốc của
libvorbisenc), **không phải** thang `-1` đến `10` mà công cụ dòng lệnh
`oggenc` hay hiển thị (`-q 4` = quality 0.4 gần đúng, không phải bằng hệt).
Nếu bạn muốn khớp chính xác với mức "Chất lượng Vorbis: 0.4" đang có trong
tool, giữ nguyên thang 0.0–1.0 là đúng rồi — không cần đổi gì.

## 3. `vorbis_analysis_wrote(&vd, 0)` để báo EOS

Đoạn code trong `aotuv_wrapper.c` xử lý EOS hơi thô (gọi
`vorbis_analysis_wrote(&vd, 0)` rồi flush thêm 1 lần). Đây là pattern phổ
biến trong ví dụ chính thức của libvorbis nhưng **nên test kỹ với file WAV
ngắn/dài khác nhau** — biên EOS là chỗ dễ sinh bug (cắt mất frame cuối, hoặc
lặp page cuối).

## 4. Chưa xử lý WAV có sample rate/channel bất thường

Wrapper giả định input luôn là PCM 16-bit. Nếu tool web hiện tại cho phép
upload WAV 24-bit hoặc 32-bit float, cần thêm bước convert trước khi gọi
hàm này (làm ở phía JS, trước khi tạo `Int16Array`).

## 5. Nếu autoconf thất bại (script `autogen.sh` không tồn tại)

Một số bản source cũ dùng sẵn `configure` đã generate, không có
`autogen.sh`. Nếu gặp lỗi "no such file", bỏ dòng gọi `autogen.sh`, chạy
thẳng `emconfigure ./configure` — nếu vẫn báo thiếu `configure`, nghĩa là
rơi vào trường hợp mục 1 (repo chỉ có patch, không có full source).

## 6. License

aoTuV dùng BSD-style license (như ghi trên trang gốc) — được phép dùng
trong dự án của bạn, kể cả thương mại, miễn giữ lại thông báo bản quyền gốc
trong mã nguồn nếu redistribute source. Không phải luật sư nên đây chỉ là
tóm tắt thông thường, không phải tư vấn pháp lý.

## Tóm lại

Bộ này là **điểm khởi đầu hợp lý** (khung build đúng chuẩn, wrapper C đúng
API), nhưng nhiều khả năng bạn sẽ cần 1-2 vòng chỉnh sửa nhỏ khi build thật
lần đầu — nhất là mục 1. Nếu gặp lỗi, gửi nguyên log terminal, mình đọc và
sửa tiếp được.
