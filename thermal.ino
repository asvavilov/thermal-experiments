#include "Adafruit_Thermal.h"
#include "SoftwareSerial.h"

// ESC/POS
// https://escpos.readthedocs.io/en/latest/commands.html#

//#include "logo.h"
//#include "anime.h"
//#include "robot.h"
//#include "john_wick_portrait_black_white_keanu_reeves_thumbnail.h"
//#include "john_wick_portrait_design_elements_black_white_handdrawn_68721.h"
//#include "mipiace_stgirl.h"
//#include "g729a6f3cb5d55cc5b6f6a7a758d116c1.h"

#define TX_PIN 11  // Пин Arduino к RX принтера
#define RX_PIN 10  // Пин Arduino к TX принтера

SoftwareSerial mySerial(RX_PIN, TX_PIN);
// см. ниже про DTR
Adafruit_Thermal printer(&mySerial, 7);

String utf8_to_cp1251(String source) {
  String target = "";
  uint8_t n;

  for (int i = 0; i < source.length(); i++) {
    n = source[i];

    // ASCII символы (латиница, цифры, знаки) — оставляем как есть
    if (n < 0x80) {
      target += (char)n;
    }
    // Символы, которые начинаются с 0xD0 (заглавные буквы и часть строчных)
    else if (n == 0xD0) {
      i++;  // Переходим ко второму байту
      n = source[i];
      if (n == 0x81) {  // Ё
        target += (char)0xA8;
      } else if (n >= 0x90 && n <= 0xBF) {
        target += (char)(n + 0x30);  // 0x90 → 0xC0 (А) и т.д.
      } else {
        target += (char)n;  // fallback
      }
    }
    // Символы, которые начинаются с 0xD1 (строчные буквы)
    else if (n == 0xD1) {
      i++;
      n = source[i];
      if (n == 0x91) {  // ё
        target += (char)0xB8;
      } else if (n >= 0x80 && n <= 0x8F) {
        target += (char)(n + 0x70);  // 0x80 → 0xF0 (р) и т.д.
      } else {
        target += (char)n;
      }
    }
  }
  return target;
}

void testFonts() {
  printer.println("");               //  Выводим пустую строку. Это рекомендуется делать для корректной работы некоторых принтеров после инициализации.
                                     //
  printer.setSize('L');              //  Устанавливаем крупный размер шрифта 'L' (Large)
  printer.println("Large");          //  Выводим текст
  printer.setSize('M');              //  Устанавливаем средний размер шрифта 'M' (Medium)
  printer.println("Medium");         //  Выводим текст
  printer.setSize('S');              //  Устанавливаем маленький размер шрифта 'S' (Small) - используется по умолчанию
  printer.println("Small");          //  Выводим текст
                                     //
  printer.justify('R');              //  Устанавливаем выравнивание текста по правому краю 'R' (Right)
  printer.println("Right");          //  Выводим текст
  printer.justify('C');              //  Устанавливаем выравнивание текста по центру 'C' (Center)
  printer.println("Center");         //  Выводим текст
  printer.justify('L');              //  Устанавливаем выравнивание текста по левому краю 'L' (Left) - используется по умолчанию
  printer.println("Left");           //  Выводим текст
                                     //
  printer.inverseOn();               //  Устанавливаем инверсию цвета текста.
  printer.println("Inverse");        //  Выводим текст
  printer.inverseOff();              //  Отменяем инверсию цвета текста.
                                     //
  printer.doubleHeightOn();          //  Устанавливаем удвоенную высоту текста.
  printer.println("Double Height");  //  Выводим текст
  printer.doubleHeightOff();         //  Отменяем удвоенную высоту текста.
                                     //
  printer.doubleWidthOn();           //  Устанавливаем удвоенную ширину текста.
  printer.println("Double Width");   //  Выводим текст
  printer.doubleWidthOff();          //  Отменяем удвоенную ширину текста.
                                     //
  printer.boldOn();                  //  Устанавливаем полужирное начертание текста.
  printer.println("Bold");           //  Выводим текст
  printer.boldOff();                 //  Отменяем полужирное начертание текста.
                                     //
  printer.underlineOn();             //  Устанавливаем подчёркивание текста линией в 1 пиксель. Равнозначно вызову функции с параметром 1: printer.underlineOn(1);
  printer.println("Underline 1px");  //  Выводим текст
  printer.underlineOn(2);            //  Устанавливаем подчёркивание текста линией в 2 пикселя.
  printer.println("Underline 2px");  //  Выводим текст
  printer.underlineOff();            //  Отменяем подчёркивание текста.
                                     //
  printer.setLineHeight(25);         //  Устанавливаем межстрочный интервал в 2,5 мм. В качестве параметра можно указать интервал от 24 (2,4 мм) до 32767. Значение по умолчанию = 30 (3,0 мм)
  printer.println("1\r\n2\r\n3");    //  Выводим 3 строки текста
  printer.setLineHeight(60);         //  Устанавливаем межстрочный интервал в 6,0 мм.
  printer.println("4\r\n5\r\n6");    //  Выводим 3 строки текста
  printer.setLineHeight();           //  Устанавливаем межстрочный в значение по умолчанию (3,0 мм)
                                     //
  printer.setCharSpacing(10);        //  Устанавливаем дополнительный межсимвольный интервал в 10 пикселов. В качестве параметра можно указать интервал от 0 до 32767 пикселей. Значение по умолчанию = 0
  printer.println("CharSpacing");    //  Выводим текст
  printer.setCharSpacing();          //  Отменяем дополнительный межсимвольный интервал. Равнозначно вызову функции с параметром 0: printer.setCharSpacing(0);
                                     //
  printer.tab();                     //  Устанавливаем отступ (табуляцию).
  printer.println("<- Tabulation");  //  Выводим текст
                                     //
  printer.feedRows(10);              //  Прокручиваем кассовую ленту на 10 пикселей. В качестве параметра можно указать от 0 до 255 пикселей.
                                     //
  if (printer.hasPaper()) {          //  Если кассовая лента установлена (есть бумага), то ...
    printer.println("Paper OK");     //  Выводим текст
  } else {                           //  Если кассовая лента отсутствует (нет бумаги), то ...
                                     //  Тут можно, например, включить внешний светодиод или звуковой сигнал сигнализирующий об окончании кассовой ленты
  }                                  //
                                     //
  printer.setDefault();              //  Устанавливаем настройки принтера по умолчанию. Функцию удобно использовать если Вы желаете вывести текст, но хотите быть уверены что на него не повлияют ранее установленные размеры или начертания.
  printer.feed(3);                   //  Прокручиваем ленту на 3 строки
}

void testHello() {
  printer.println("Hello, TTL Printer!");

  printer.feed(2);

  printer.setCodePage(CODEPAGE_WCP1251);

  uint8_t message[] = { 0xCF, 0xF0, 0xE8, 0xE2, 0xE5, 0xF2, 0x00 };
  printer.print((char*)message);
  printer.println();

  printer.println(utf8_to_cp1251("Привет, мир!"));
  printer.println(utf8_to_cp1251("Это вторая строка"));
  printer.println(utf8_to_cp1251("Съешь ещё этих мягких французских булок"));

  /*
  Данные для печати поступают сначала в буфер принтера, а уже из него берутся для формирования изображений на кассовой ленте.
  Принтер не сообщает информацию об уровне заполненности буфера входных данных, по этому библиотека делает паузы на время требуемое для печати.
  Дело в том что скорость передачи данных по шине UART выше чем скорость печати принтера.
  Если отправлять данные потоком, без пауз, то буфер входных данных принтера может переполнится, что приведёт к потере текста или искажению изображений.
  С другой стороны, паузы устанавливаемые библиотекой сильно увеличивают время печати.

  Выходом из данной ситуации может стать подключение жёлтого провода DTR (Data Terminal Ready) шины UART к любому выводу Arduino.
  Так принтер сможет сообщать о готовности получения новых данных.
  Номер вывода Arduino к которому подключён желтый провод нужно указать в качестве второго аргумента при объявлении объекта библиотеки Adafruit_Thermal.

  Adafruit_Thermal printer(&mySerial,7); //  Объявляем объект printer с указанием ссылки на объект mySerial (или на класс Serial) и № любого вывода Arduino (в данном случае 7) к которому подключен вывод DTR принтера

  Теперь принтер будет сообщать библиотеке о готовности принять новые данные, а библиотека не будет устанавливать дополнительные паузы, это в разы ускорит работу принтера.
  */
  // Проверено: нормально печается через функцию-конвертер
  /*
  Но вывод русского текста будет возможен только посимвольно, через функцию write().
  // Вариант через цикл с write()
  uint8_t message[] = {0xCF, 0xF0, 0xE8, 0xE2, 0xE5, 0xF2}; // "Привет" без нулевого терминатора
  // Отправляем каждый байт через write()
  for(int i = 0; i < sizeof(message); i++) {
    printer.write(message[i]);
  }
  printer.println(); // Переход на новую строку
  */

  printer.feed(2);
}

// бинарные чб изображения (0 - нет пикселя, 1 - есть пиксель)
// LCDAssistant
// https://impressto.ca/image2hex.php (https://github.com/impressto/image_to_bw_hex)
// https://notisrac.github.io/FileToCArray/
void testBitmap() {
  //printer.printBitmap(32, 20, logo);  //  Печать изображения (логотипа iarduino) размером 32x20 пикселей из массива logo
  //printer.feed(1);
  //printer.printBitmap(49, 64, bitmap_anime);
  //printer.feed(1);
  //printer.printBitmap(240, 240, robot);
  //printer.feed(1);
  //printer.printBitmap(360, 414, png_transparent_john_wick_portrait_black_white_keanu_reeves_thumbnail);
  //printer.feed(1);
  //printer.printBitmap(268, 268, john_wick_portrait_design_elements_black_white_handdrawn_68721);
  //printer.feed(1);
  //printer.printBitmap(400, 425, mipiace_stgirl);
  //printer.feed(1);
  //printer.printBitmap(474, 474, g729a6f3cb5d55cc5b6f6a7a758d116c1);

  //printer.feed(2);
}

void testBars() {
  printer.print(F("UPC-A:"));                    //  Выводим текст с названием формата штрих-кода
  printer.printBarcode("123456789012", UPC_A);   //  Выводим штрих-код в формате UPC_A.                                В данном формате используются 12 цифр
                                                 //
  printer.print(F("EAN-13:"));                   //  Выводим текст с названием формата штрих-кода
  printer.printBarcode("1234567890123", EAN13);  //  Выводим штрих-код в формате EAN-13, аналогичен формату JAN-13.    В данном формате используются 13 цифр
                                                 //
  printer.print(F("EAN-8:"));                    //  Выводим текст с названием формата штрих-кода
  printer.printBarcode("12345678", EAN8);        //  Выводим штрих-код в формате EAN-8, аналогичен формату JAN-8.      В данном формате используются 8 цифр
                                                 //
  printer.print(F("CODE 39:"));                  //  Выводим текст с названием формата штрих-кода
  printer.printBarcode("IARDUINO", CODE39);      //  Выводим штрих-код в формате CODE 39                               В данном формате можно использовать от 2 до 255 цифр и заглавных латинских букв
                                                 //
  printer.print(F("CODE 93:"));                  //  Выводим текст с названием формата штрих-кода
  printer.printBarcode("IARDUINO.RU", CODE93);   //  Выводим штрих-код в формате CODE 93                               В данном формате можно использовать от 2 до 255 цифр, заглавных латинских букв, пробелы, и знаки *.:$%/+-
                                                 //
  printer.print(F("ITF:"));                      //  Выводим текст с названием формата штрих-кода
  printer.printBarcode("1234567890", ITF);       //  Выводим штрих-код в формате ITF                                   В данном формате можно использовать от 2 до 255 цифр
                                                 //
  printer.print(F("CODABAR:"));                  //  Выводим текст с названием формата штрих-кода
  printer.printBarcode("1234567890", CODABAR);   //  Выводим штрих-код в формате CODABAR                               В данном формате можно использовать от 2 до 255 цифр, заглавных латинских букв, и знаки .:%/+-
                                                 //
  printer.print(F("CODE128:"));                  //  Выводим текст с названием формата штрих-кода
  printer.printBarcode("iArduino", CODE128);     //  Выводим штрих-код в формате CODE 128                              В данном формате можно использовать от 2 до 255 символов коды которых лежат в диапазоне от 0 до 127 таблицы символов ASCII
                                                 //
  printer.feed(2);
}

void serialTest() {
  // ESC @ - Сброс принтера
  mySerial.write(0x1B);
  mySerial.write(0x40);

  // Отправка текста (ASCII символы)
  mySerial.write("Test send to serial\n");

  // Перевод строки (0x0A)
  mySerial.write(0x0A);
  mySerial.write(0x0A);
}

void setup() {
  mySerial.begin(9600);  // Убедитесь, что скорость совпадает с настройкой принтера
                         //  Инициируем передачу данных по программной шине UART на скорости 9600. Функцию begin объекта mySerial нужно вызвать до вызова функции begin объекта printer!
  printer.begin();       //  Инициируем работу с термопринтером. В качестве параметра можно указать время нагрева пикселей от 3 (0,03 мс) до 255 (2,55 мс), чем выше тем темнее пикселы. Значение по умолчанию = 120 (1,20 мс)
  // NOTE в этом случае printer.begin() не нужен
  //testSerial();

  //testHello();

  //testBitmap();

  //testFonts();

  //testBars();

  //printer.test(); // выводит Hello World!

  //printer.testPage(); // выводит тестовую страницу
}

void loop() {}
