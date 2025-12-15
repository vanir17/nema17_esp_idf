// ============================================================
// GÁ KẸP BÀN ĐỘNG CƠ NEMA 17 (NẰM NGANG)
// Thiết kế tối ưu cho in 3D (không cần support khi đặt nằm nghiêng)
// ============================================================

/* --- HƯỚNG DẪN ---
1. Điều chỉnh thông số "ban_day_max" cho phù hợp với bàn của bạn.
2. Khi in 3D: Xoay vật thể 90 độ quanh trục Y (để mặt bên áp xuống bàn in).
   Điều này giúp in không cần support và kết cấu cứng nhất.
3. Bạn cần chuẩn bị:
   - 4 ốc M3 ngắn để bắt motor vào gá.
   - 1 bu lông lớn (M8 hoặc M10) + đai ốc để làm chân kẹp.
*/

// --- THÔNG SỐ CẤU HÌNH (Đơn vị: mm) ---

// Thông số bàn và kẹp
ban_day_max = 35;       // Độ dày tối đa của mặt bàn có thể kẹp
do_day_tuong = 6;       // Độ dày của thành gá (nên để >= 5mm cho cứng)
be_rong_ga = 50;        // Bề rộng tổng thể của gá
duong_kinh_oc_kep = 8.5; // Đường kính lỗ cho ốc kẹp (Ví dụ M8 thì để 8.5mm cho lỏng)

// Thông số chuẩn NEMA 17 (Không nên sửa trừ khi dùng motor lạ)
nema_side = 42.3;       // Kích thước cạnh mặt bích motor
lo_bat_oc_motor = 31;   // Khoảng cách tâm 2 lỗ ốc đối diện (chéo) là 43.8, cạnh là 31
duong_kinh_hub = 23;    // Đường kính lỗ thoát gờ định vị giữa motor (chuẩn 22mm, để 23 cho lỏng)
duong_kinh_oc_m3 = 3.4; // Lỗ xỏ vừa ốc M3

$fn = 60; // Độ mịn đường tròn

// ============================================================
// MODULE CHÍNH
// ============================================================

module ga_kep_ban_nema17() {
    // Tính toán các kích thước bao
    chieu_cao_tong = ban_day_max + 2 * do_day_tuong;
    chieu_sau_kep = nema_side + 15; // Phần ăn vào mặt bàn

    difference() {
        // 1. Khối vật liệu chính (Hình chữ C)
        union() {
            // Khối thân chính
            cube([be_rong_ga, chieu_sau_kep + do_day_tuong, chieu_cao_tong]);
            
            // Khối mặt bích để gắn motor (phía trước)
            translate([0, -do_day_tuong, do_day_tuong]) {
                // Tạo khối vuông vức trước
                 cube([be_rong_ga, do_day_tuong, nema_side + 5]);
            }
            
             // Bo tròn góc chịu lực (Fillet) giữa mặt gắn motor và thân kẹp
             // Đây là phần quan trọng để tránh bị gãy khi siết ốc
            translate([0, do_day_tuong, do_day_tuong + ban_day_max])
                rotate([0, 90, 0])
                fillet_concave(r=do_day_tuong, l=be_rong_ga);
             
            translate([0, do_day_tuong, do_day_tuong])
                rotate([0, 90, 0])
                rotate([0,0,-90])
                fillet_concave(r=do_day_tuong, l=be_rong_ga);
        }

        // --- CÁC PHẦN CẮT BỎ ---
        
        // 2. Khe hở để đút vào mặt bàn
        translate([-1, do_day_tuong, do_day_tuong])
            cube([be_rong_ga + 2, chieu_sau_kep + 1, ban_day_max]);

        // 3. Các lỗ bắt động cơ NEMA 17 (Ở mặt trước)
        translate([be_rong_ga/2, -do_day_tuong/2, do_day_tuong + nema_side/2 + 2.5])
        rotate([90, 0, 0]) // Xoay để đục lỗ nằm ngang
        {
            // Lỗ Hub giữa
            cylinder(h = do_day_tuong + 2, d = duong_kinh_hub, center=true);
            
            // 4 lỗ ốc M3 ở góc
            for (x_pos = [-1, 1]) {
                for (y_pos = [-1, 1]) {
                    translate([x_pos * lo_bat_oc_motor/2, y_pos * lo_bat_oc_motor/2, 0])
                        cylinder(h = do_day_tuong + 2, d = duong_kinh_oc_m3, center=true);
                }
            }
        }
        
        // 4. Lỗ cho bu lông kẹp bàn (Ở mặt dưới)
        // Vị trí: nằm giữa bề rộng, và cách mép trong một khoảng
        translate([be_rong_ga/2, do_day_tuong + chieu_sau_kep/2 , -1])
            cylinder(h = do_day_tuong + 2, d = duong_kinh_oc_kep);
    }
}

// Module phụ trợ để tạo góc bo tròn âm (Concave Fillet)
// Giúp tăng cường độ cứng vững cho các góc vuông
module fillet_concave(r, l) {
    difference() {
        cube([r, r, l]);
        translate([r, r, -1])
            cylinder(h=l+2, r=r);
    }
}


// === GỌI MODULE ĐỂ HIỂN THỊ ===

// Gợi ý: Bỏ comment dòng rotate bên dưới để xem hướng in 3D tối ưu
// rotate([0, -90, 0]) 
ga_kep_ban_nema17();

// Bonus: Một miếng đệm nhỏ để lót đầu ốc kẹp, tránh làm hỏng mặt bàn
/*
translate([be_rong_ga + 20, 0, 0])
difference() {
    cylinder(h=3, d=20); // Miếng đệm tròn đường kính 20mm
    translate([0,0,1.5])
        cylinder(h=2, d=duong_kinh_oc_kep+0.5, $fn=6); // Lỗ lục giác để giữ đầu đai ốc (nếu cần)
}
*/