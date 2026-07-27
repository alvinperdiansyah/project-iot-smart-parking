Deskripsi Proyek

Smart Parking Gate Berbasis ESP32 dengan RFID, Sensor Ultrasonik, dan Telegram

Proyek ini merupakan sistem gerbang parkir otomatis (Smart Parking Gate) yang dirancang menggunakan mikrokontroler ESP32. Sistem menggabungkan teknologi RFID, sensor ultrasonik, servo motor, serta bot Telegram untuk mengontrol akses kendaraan secara otomatis dan memantau kondisi gerbang secara real-time melalui internet.

Pengguna yang memiliki kartu RFID terdaftar dapat membuka gerbang secara otomatis. Sebelum gerbang dibuka, sistem memastikan adanya kendaraan di depan gerbang menggunakan sensor ultrasonik sehingga gerbang tidak terbuka jika tidak ada kendaraan. Setelah kendaraan melewati gerbang, sistem akan menutup gerbang secara otomatis berdasarkan pembacaan sensor keluar.

Untuk meningkatkan keamanan, sistem dilengkapi dengan fitur pendeteksi penerobos. Apabila terdeteksi kendaraan atau objek yang mencoba melewati gerbang tanpa izin, buzzer akan berbunyi sebagai alarm, LED indikator berubah menjadi merah, dan notifikasi akan dikirimkan ke Telegram. Selain itu, sistem juga memiliki mekanisme timeout yang akan menutup gerbang secara otomatis apabila kendaraan tidak segera melewati gerbang setelah dibuka.

ESP32 terhubung ke jaringan Wi-Fi sehingga dapat berkomunikasi dengan Bot Telegram. Melalui Telegram, pengguna dapat menerima notifikasi berbagai kondisi sistem, seperti gerbang dibuka, gerbang ditutup, akses RFID ditolak, maupun adanya percobaan penerobosan. Pengguna juga dapat mengendalikan gerbang dari jarak jauh menggunakan perintah /buka, /tutup, dan /status.

Secara keseluruhan, proyek ini bertujuan untuk meningkatkan keamanan dan efisiensi sistem parkir dengan mengotomatisasi proses keluar-masuk kendaraan serta menyediakan pemantauan dan pengendalian secara real-time melalui internet.

Fitur Utama
Autentikasi akses menggunakan kartu RFID.
Deteksi keberadaan kendaraan menggunakan dua sensor ultrasonik.
Kontrol buka dan tutup gerbang menggunakan servo motor.
Indikator status menggunakan LED merah dan hijau.
Alarm buzzer untuk akses tidak sah dan deteksi penerobos.
Notifikasi otomatis ke Telegram untuk setiap kejadian penting.
Kontrol gerbang dari Telegram melalui perintah /buka, /tutup, dan /status.
Penutupan gerbang otomatis setelah kendaraan melewati gerbang.
Mekanisme timeout untuk mencegah gerbang terbuka terlalu lama.
Monitoring status sistem secara real-time melalui koneksi Wi-Fi.
