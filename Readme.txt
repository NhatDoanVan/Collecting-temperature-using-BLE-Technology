##Project Nhóm 8 : Hệ thống thu giám sát nhiệt độ từ xa vườn ươm trong nhà kính
  Miêu tả: Đây là một mô hình thu thập theo dõi nhiệt độ từ xa cho các cây giống trong vườn ươm để kịp tiến hành quản lý chăm sóc cho 
cây giống. Mô hình hiển thị nhiệt độ thời gian thực trên ThingsBoard, cập nhập thời gian lấy mẫu từ xa, tình trạng kết nối node,..
################################################################# 

##Mã code sử dụng thư viện ESP-idf
 Hướng dẫn cài tool để lạp trình và sử dụng mã code:

  B1: Cài đặt Git và Python 3
Nếu bạn chưa cài đặt Git và Python 3, hãy thực hiện các bước sau:

Đối với hệ điều hành Windows:

+, Tải xuống và cài đặt Git từ trang web https://git-scm.com/download/win.
+, Tải xuống và cài đặt Python 3 từ trang web https://www.python.org/downloads/windows/.

Đối với hệ điều hành Linux:
 - Mở Terminal và chạy lệnh sau để cài đặt Git:
     sudo apt-get install git
 - Chạy lệnh sau để cài đặt Python 3:
     sudo apt-get install python3
  B2: Tải ESP-IDF
 - Truy cập trang web chính thức của Espressif Systems để tải ESP-IDF về máy tính của bạn. Để tải xuống phiên bản mới nhất của ESP-IDF, 
bạn có thể sử dụng lệnh sau trong Terminal:
    git clone --recursive https://github.com/espressif/esp-idf.git
 - Lưu ý rằng nếu bạn muốn tải xuống một phiên bản cụ thể của ESP-IDF, bạn có thể chuyển sang phiên bản đó bằng lệnh sau:
     git checkout <tag>
Trong đó, <tag> là phiên bản của ESP-IDF mà bạn muốn chuyển đến.

  B3:Giải nén ESP-IDF và cài đặt các công cụ lệnh
  - Sau khi tải xuống ESP-IDF, bạn cần giải nén tệp nén bằng cách chạy lệnh sau trong Terminal:
cd esp-idf
tar -xzf esp-idf-v4.4.tar.gz
  - Sau khi giải nén ESP-IDF, bạn cần chạy lệnh sau để cài đặt các công cụ lệnh:
./install.sh
   B4: Cấu hình môi trường
 - Để cấu hình môi trường cho ESP-IDF, bạn cần chạy lệnh sau trong Terminal:
. $HOME/esp/esp-idf/export.sh
Lưu ý rằng $HOME/esp là đường dẫn đến thư mục ESP-IDF trên máy tính của bạn.

   B5:Cài đặt driver cho ESP32
Để kết nối ESP32 với máy tính, bạn cần cài đặt driver cho ESP32. Truy cập trang web của Espressif Systems để tải xuống và 
cài đặt driver cho ESP32.Sau khi hoàn thành các bước trên, bạn đã cài đặt thành công ESP-IDF cho ESP32 trên máy tính

#########################################################
  Hướng dẫn biên dịch và nạp code cho gateway và node cảm biến 
 B1: Bạn download file mã code EE4552-G08-Final-LMT.zip tiến hành giải nén trong file sẽ chứa ba file node cảm biến A,B,C
và flie gateway. Bạn tiến hành lưu nó vào đường dẫn sau:$HOME/esp/gateway
Trong đó $HOME/esp là đường dẫn đến thư mục ESP-IDF trên máy tính của bạn. 
 Các file node làm tương tự.
B2: Để có thể sử code bạn có thể cài đặt Visual Studio Code để biện dịch và thay đổi code theo mong muôns
B3: Để nạp chương trình vào esp32. Bạn mở ESP-IDF 4.3 PowerShell ở Desktop
chạy lệnh: cd.. để ra đường dẫn $HOME/esp . Sau đó chạy lệnh: ls để xem toàn bộ file code được liệt kệ trong thư mục $HOME/esp đã lưu phía 
B1. Tiếp chạy lệnh: cd <tag> 
Trong đó  <tag> là tên thư mục bạn muốn vào để xem hoặc chỉnh sủa code. Ví dụ cd gateway
+,Để biên dịch lại code bạn chạy lệnh: idf.py build
+,Để mở mã code bạn chạy lệnh: code
+,Để nạp chượng trình bạn chạy mã: idf.py -p <port> flash .Dùng để nạp chương trình vào ESP32 thông qua cổng kết nối.
 Thay <port> bằng tên cổng kết nối (ví dụ: /dev/ttyUSB0 trên Linux hoặc COM3 trên Windows)
Lưu ý ở mã code gateway cần cấu hình lại wifi mà bạn muốn dùng đẻ đẩy dữ liệu lên cloud. Để làm đc điều đó khi ở đường dẫn :$HOME/esp/gateway
Chạy lệnh: idf.py menuconfig . Sau đó thay đổi tên đăng nhập và mật khẩu mà wifi bạn dùng, nhấn phím ESC rồi Y để lưu hay đổi rồi tiến hành 
nạp code bằng câu lệnh ở hướng dẫn trên.