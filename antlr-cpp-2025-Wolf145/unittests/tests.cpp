#include <gtest/gtest.h>
#include <libitmoantlr/Translator.h>

#include "test_helper.h"

TEST(ANTLRTests, TestExample) {
  Translator tr;
  std::string c_code = tr.translate("program p; const Pi = 3.14; Count = Pi; Name = 'Mike'; var a: integer; b:string; begin read(b); read(a); readln(a); write(a); write(Name); writeln(); end.");
  test_helper::write_sample(c_code, "");
}

TEST(ANTLRTests, TestTypeDef) {
  Translator tr;
  std::string c_code = tr.translate("program bibaboba; type TColor = (Red, Green, Blue); TPoint = record x, y: Integer;end; TPerson = record name: TString20; age: TSmallInt; favorite: TColor; pos: TPoint;end;begin end.");
  test_helper::write_sample(c_code, "");
}

/*
program bibaboba;
type
 TColor = (Red, Green, Blue);
 TPoint = record
  x, y: Integer;
 end;
 TPerson = record
  name: TString20;
  age: TSmallInt;
  favorite: TColor;
  pos: TPoint;
  end;
begin
end.
*/

TEST(ANTLRTests, TestRecordArrayRecord) {
  Translator tr;
  std::string c_code = tr.translate("program bibaboba;type TContainer = record count: Integer; items: array[0..4, 0..6] of record  id: Integer;  value: Real;  end; end;begin end.");
  test_helper::write_sample(c_code, "");
}

/*
program bibaboba;
type
  TContainer = record
    count: Integer;
    items: array[0..4, 0..6] of record
      id: Integer;
      value: Real;
    end;
  end;
begin
end.
*/

TEST(ANTLRTests, TestFunction) {
  Translator tr;
  std::string c_code = tr.translate("program bibaboba; type aboba = (ZELIBOBA, ABOBA, SUS); function sisipisi(a, b: Integer; c: aboba): Integer;var A, B, C, sum, prod, divis:Integer; divisf : real; const P = c; begin sum := a + b; prod := a * b;  divis := a div b; divisf := a / b; sisipisi := sum * prod + divis; end; procedure sss(); const s = 4; begin s := s + sisipisi(1, 2, 3, 4, 5+1000); end;begin end.");
  test_helper::write_sample(c_code, "");
}

TEST(ANTLRTests, TestSwitch) {
  Translator tr;
  std::string c_code = tr.translate("case x of 1, 2, 3: y := 10; 6: y := 6; 7: y := 7 end;", [](GrammarParser &p) {
        return p.switchStatement();
    });
  test_helper::write_sample(c_code, "");
}
/*
case x of
  1, 2, 3: y := 10;
  6: y := 6;
  7: y := 7;
end;
*/

TEST(ANTLRTests, GCD) {
  Translator tr;
  std::string c_code = tr.translate("program GCD_Euclid; var a, b, t: Integer; begin readln(a, b); if a < 0 then a := -a; if b < 0 then b := -b; while b <> 0 do begin t := a mod b; a := b; b := t; end; writeln(a); end.");
  test_helper::write_sample(c_code, "");
}

TEST(ANTLRTests, IsPrime) {
  Translator tr;
  std::string c_code = tr.translate("program CheckPrime; function IsPrime(n: Integer): Boolean; var i: Integer; begin if n < 2 then IsPrime := false else begin IsPrime := true; for i := 2 to trunc(sqrt(n)) do if n mod i = 0 then begin IsPrime := false; break; end; end; end; var n: Integer; begin readln(n); if IsPrime(n) then writeln('YES') else writeln('NO'); end.");
  test_helper::write_sample(c_code, "");
}

TEST(ANTLRTests, OneMoreTest) {
  Translator tr;
  std::string c_code = tr.translate("program T11; var a: array[1..2,1..3] of integer; i,j: integer; s: integer; begin s:=0; for i:=1 to 2 do for j:=1 to 3 do begin read(a[i,j]); s:=s+a[i,j]; end; writeln(s); end.");
  test_helper::write_sample(c_code, "");
}

TEST(ANTLRTests, TestRef) {
  Translator tr;
  std::string c_code = tr.translate("function e(var a, b : Integer; c, d : Real) : Real; begin end;", [](GrammarParser &p) {
        return p.procedureAndFunctionDeclarationPart();
    });
  test_helper::write_sample(c_code, "");
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
