# Cập nhật lần 2 — thêm chế độ bitrate-managed

File `src/aotuv_wrapper.c` đã được sửa: hàm `aotuv_encode_wav_to_ogg` giờ có
thêm 2 tham số `mode` và `nominal_bitrate`, để thử chế độ khởi tạo encoder
theo bitrate quản lý (ABR) thay vì theo quality (VBR) — xem giải thích lý
do trong comment đầu hàm.

## Cách push đè lên repo cũ

KHÔNG cần tạo repo mới, KHÔNG cần `git init` lại. Chỉ cần thay 2 file:
- `src/aotuv_wrapper.c`
- `js/example_usage.js`

vào đúng vị trí cũ trong thư mục `aotuv-wasm-build` trên máy bạn (ghi đè),
rồi trong cmd, đứng tại đúng thư mục đó:

```cmd
git add .
git commit -m "Add bitrate-managed encode mode"
git push
```

GitHub Actions sẽ tự chạy lại. Vào tab Actions xem, đợi xong tải Artifact
`aotuv-wasm` mới về gửi lại cho mình — mình sẽ nhúng vào tool và chỉnh JS
gọi đúng tham số mới (mode=1, nominal_bitrate) để test thử.

## Bitrate thử đầu tiên

Log lần trước cho thấy nominal bitrate ước tính khoảng 128000 bps (128kbps)
ở quality 0.4 — mình sẽ dùng số này làm điểm khởi đầu để test mode=1, có
thể cần thử thêm vài giá trị khác nếu không khớp ngay.
