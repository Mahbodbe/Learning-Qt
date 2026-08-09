# 🛠️ یادگیری Qt: توسعه رابط گرافیکی و پروژه‌های تمرینی

> **منبع اصلی:** کتاب *Getting Started with Qt 5* نوشته **بنجامین باکا (Benjamin Baka)**  
> **نویسنده:** مهبد بمانی‌چم ([@Mahbodbe](https://github.com/Mahbodbe))  

---

<p align="center">
  <a href="README.md">
    <img src="https://img.shields.io/badge/Language-English-blue.svg?style=flat-square" alt="English">
  </a>
  <a href="README_FA.md">
    <img src="https://img.shields.io/badge/Language-%D9%81%D8%A7%D8%B1%D8%B3%DB%8C-green.svg?style=flat-square" alt="فارسی">
  </a>
</p>

---

## 📌 درباره این ریپوزیتوری
این ریپوزیتوری مسیر یادگیری و تمرینات عملی من در فریم‌ورک **Qt C++** و نرم‌افزار **Qt Creator** را مستند می‌کند.

محتوای این ریپوزیتوری شامل نمونه کدها، تمرینات طبقه‌بندی‌شده و پروژه‌هایی است که از منابع زیر استخراج شده‌اند:
1. **مرجع اصلی:** کتاب *Getting Started with Qt 5* نوشته **بنجامین باکا (Benjamin Baka)**.
2. **تمرینات هوش مصنوعی (AI):** تمرینات اختصاصی که توسط دستیار هوش مصنوعی برای تسلط بر مفاهیم فریم‌ورک طراحی شده‌اند (مانند کار با ویزارد Qt Creator، سیگنال‌ها و اسلات‌ها، چیدمان‌ها (Layouts) و ویجت‌های سفارشی).

> [!NOTE]
> با اینکه بخشی از کدها بر اساس فصل‌های کتاب پیش می‌روند، اما الزماً محدود به آن نیست و شامل تمرینات اضافی و آزمایش‌های مستقل (مانند اولین تمرین کار با ویزارد در پوشه `first`) نیز می‌شود.

---

## 📂 ساختار ریپوزیتوری

```text
Learning-Qt/
├── first/            # اولین تمرین عملی کار با ویزارد (Qt Widgets & Qt Creator)
├── second/           # تمرینات چیدمان‌ها، ویجت‌ها و سیستم سیگنال و اسلات در Qt
├── third/            # برنامه‌های چندپنچره‌ای، دیالوگ‌ها و کامپوننت‌های پیشرفته UI
├── .gitignore        # فایل‌های نادیده‌گرفته‌شده Qt Creator و کامپایلر
└── README.md         # مستندات پروژه (انگلیسی دیفالت) و README_FA.md (فارسی)
```

---

## 🚀 راهنمای اجرا

### پیش‌نیازها
- **Qt Creator** (نسخه ۴ به بالا یا ۱۰ به بالا)
- **Qt Framework** (نسخه Qt 5 یا Qt 6)
- **کامپایلر C++** (GCC / Clang / MSVC)
- **QMake** یا **CMake**

### نحوه ساخت و اجرا
1. کلون کردن ریپوزیتوری:
   ```bash
   git clone https://github.com/Mahbodbe/Learning-Qt.git
   ```
2. اجرای **Qt Creator**.
3. از منوی بالا گزینه‌ی `File -> Open File or Project...` را زده و فایل `.pro` یا `CMakeLists.txt` مربوط به هر تمرین را باز کنید (مثلاً `first/first.pro`).
4. دکمه **Configure Project** را زده و سپس با کلیدهای ترکیب `Ctrl + R` برنامه را اجرا کنید.

---

## 👤 نویسنده

* **مهبد بمانی‌چم** — [@Mahbodbe](https://github.com/Mahbodbe)
<!-- fa update -->
