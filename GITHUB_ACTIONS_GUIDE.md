# Build aotuv.wasm bằng GitHub Actions (không cần cài Docker)

Máy tính chỉ cần cài **Git** (nhẹ, không cần bật ảo hóa/WSL2 gì cả). Toàn bộ
việc build Docker thật sự diễn ra trên server của GitHub, miễn phí.

## Bước 1 — Tạo tài khoản GitHub (nếu chưa có)

Vào https://github.com/signup, đăng ký miễn phí.

## Bước 2 — Cài Git cho Windows

Tải tại: https://git-scm.com/download/win
Cài đặt theo mặc định (Next liên tục), không cần chỉnh gì.

Sau khi cài xong, mở **cmd mới** (đóng cmd cũ đi, mở lại), gõ thử:
```
git --version
```
Nếu hiện ra số phiên bản (vd `git version 2.45.0`) là cài thành công.

## Bước 3 — Tạo repository trên GitHub

1. Vào https://github.com/new
2. Đặt tên repo, ví dụ: `aotuv-wasm-build`
3. Chọn **Public** (bắt buộc để dùng GitHub Actions miễn phí không giới hạn;
   Private cũng có free tier nhưng có giới hạn phút chạy/tháng)
4. **Không** tick "Add a README file" (vì mình đã có sẵn)
5. Bấm "Create repository"
6. GitHub sẽ hiện ra 1 trang có đoạn URL dạng:
   `https://github.com/<ten_ban>/aotuv-wasm-build.git` — giữ lại trang này.

## Bước 4 — Push code lên GitHub từ cmd

Trong cmd, đứng tại đúng thư mục `aotuv-wasm-build` (thư mục đã giải nén
từ file zip mình gửi, đã có sẵn `.github/workflows/build.yml`):

```cmd
cd C:\Users\Acer\Downloads\aotuv-wasm-build
git init
git add .
git commit -m "Initial commit"
git branch -M main
git remote add origin https://github.com/<ten_ban>/aotuv-wasm-build.git
git push -u origin main
```

Thay `<ten_ban>` bằng username GitHub thật của bạn.

Lần đầu push, Git sẽ mở popup yêu cầu đăng nhập GitHub qua trình duyệt —
đăng nhập bình thường là xong, không cần nhập token thủ công.

## Bước 5 — Xem build chạy tự động

1. Vào lại trang repo trên GitHub: `https://github.com/<ten_ban>/aotuv-wasm-build`
2. Bấm tab **Actions** (trên thanh menu, cạnh "Code", "Issues"...)
3. Sẽ thấy 1 job tên "Build aoTuV WASM" đang chạy (chấm vàng xoay) hoặc đã
   chạy xong (dấu tích xanh ✓ hoặc dấu X đỏ nếu lỗi)
4. Bấm vào job đó để xem log chi tiết từng bước (giống hệt log docker mà
   bạn đã thấy trên cmd/termux trước đây)

Build mất khoảng 15-30 phút cho lần đầu (tải Emscripten image ~1-2GB).

## Bước 6 — Tải file aotuv.wasm về

Nếu job chạy xong với dấu tích xanh:
1. Vẫn ở trang job đó, kéo xuống cuối trang
2. Sẽ thấy mục **Artifacts** với 1 file tên `aotuv-wasm`
3. Bấm vào để tải về — đây là file `.zip` chứa `aotuv.wasm` + `aotuv.js`

Nếu job báo lỗi (dấu X đỏ): bấm vào bước bị lỗi trong log, copy toàn bộ nội
dung log đó gửi lại cho mình để đọc và sửa `Dockerfile`/`build_all.sh`.

## Bước 7 — Sửa lại rồi push lại (nếu cần)

Nếu cần sửa code (vd sau khi mình gửi bản vá lỗi), lặp lại từ bước sửa file
xong thì:
```cmd
git add .
git commit -m "Fix build error"
git push
```
GitHub sẽ tự chạy lại workflow mỗi lần push — không cần lặp lại từ đầu.

## Sau khi có aotuv.wasm

Copy 2 file `aotuv.js` + `aotuv.wasm` vào cùng thư mục với tool web
(`wav_to_wem-2.html`) của bạn, làm theo phần "7. Copy vào tool web" trong
README.md gốc.
