
#include <hobbes/hobbes.H>
#include "test.H"

using namespace hobbes;
static cc& c() { static cc x; return x; }

TEST(Matching, Basic) {
  EXPECT_EQ(c().compileFn<int()>("match 1 2 with | 1 2 -> 1 | _ 2 -> 2 | _ _ -> 3")(), 1);
  EXPECT_EQ(c().compileFn<int()>("match 2 2 with | 1 2 -> 1 | _ 2 -> 2 | _ _ -> 3")(), 2);
  EXPECT_EQ(c().compileFn<int()>("match 2 3 with | 1 2 -> 1 | _ 2 -> 2 | _ _ -> 3")(), 3);
  EXPECT_EQ(c().compileFn<int()>("match 2 9 with | 1 2 -> 1 | 2 x -> x | _ _ -> 3")(), 9);

  EXPECT_EQ(c().compileFn<int()>("match (+) 1 2 with | f x y -> f(x,y)")(), 3);
  EXPECT_EQ(c().compileFn<int()>("let (x, y) = (1, 2) in x + y")(), 3);
}

TEST(Matching, Strings) {
  EXPECT_EQ(c().compileFn<int()>("match \"foo\" with | \"fox\" -> 1 | \"for\" -> 2 | _ -> 3")(), 3);

  // verify matching in std::string values (array matching should be overloaded)
  static std::string stdpatstr = "hello";
  c().bind("stdpatstr", &stdpatstr);
  EXPECT_EQ(c().compileFn<int()>("match stdpatstr with | \"hello\" -> 0 | _ -> 9")(), 0);
  EXPECT_EQ(c().compileFn<int()>("match stdpatstr with | \"hell\" -> 0 | _ -> 9")(), 9);

  EXPECT_EQ(c().compileFn<int()>("match \"abc\" 2 with | _ 2 -> 1 | \"abc\" _ -> 2 | _ 3 -> 3 | _ _ -> 4")(), 1);
  EXPECT_EQ(c().compileFn<int()>("match \"abc\" 3 with | _ 2 -> 1 | \"abc\" _ -> 2 | _ 3 -> 3 | _ _ -> 4")(), 2);
  EXPECT_EQ(c().compileFn<int()>("match \"abd\" 3 with | _ 2 -> 1 | \"abc\" _ -> 2 | _ 3 -> 3 | _ _ -> 4")(), 3);
  EXPECT_EQ(c().compileFn<int()>("match \"abd\" 4 with | _ 2 -> 1 | \"abc\" _ -> 2 | _ 3 -> 3 | _ _ -> 4")(), 4);

  EXPECT_EQ(
    c().compileFn<int()>(
      "match \"abc\" \"three\" with | _ \"two\" -> 1 | \"abc\" _ -> 2 | _ \"three\" -> 3 | _ _ -> 4"
    )(),
    2
  );

  EXPECT_TRUE(
    c().compileFn<bool()>(
      "let f = (\\x y z.match x y z with | \"aaa\" \"bbb\" _ -> 0 | \"aaa\" \"bbc\" \"ccc\" -> 1 | _ _ _ -> 2) :: ([char],[char],[char])->int in "
      "(f(\"aaa\",\"bbb\",\"ccc\") == 0 and f(\"aaa\",\"bbc\",\"ccc\") == 1 and f(\"aaa\",\"bbc\",\"ccd\") == 2 and f(\"aba\",\"bbb\",\"ccdaa\") == 2)"
    )()
  );

  EXPECT_EQ(
    c().compileFn<int()>(
      "((\\a b c.match a b c with | \"aaa\" \"bbb\" \"ccc\" -> 0 | \"aaa\" _ \"ccc\" -> 1 | _ _ _ -> -1) :: ([char],[char],[char]) -> int)(\"aaa\", \"ddd\", \"ccc\")"
    )(),
    1
  );
}

TEST(Matching, Arrays) {
  EXPECT_EQ(c().compileFn<int()>("match [1,2,3] with | [1,2,_] -> 1 | [1,2] -> 2 | _ -> 3")(), 1);
  EXPECT_EQ(c().compileFn<int()>("match [[1],[2]] with | [_,[2]] -> 0 | [[1],_] -> 1 | _ -> 2")(), 0);
  EXPECT_EQ(c().compileFn<int()>("match [[1],[3]] with | [_,[2]] -> 0 | [[1],_] -> 1 | _ -> 2")(), 1);
  EXPECT_EQ(c().compileFn<int()>("match [[3],[3]] with | [_,[2]] -> 0 | [[1],_] -> 1 | _ -> 2")(), 2);
}

TEST(Matching, Struct) {
  EXPECT_EQ(c().compileFn<int()>("match (2,2) with | (1,2) -> 1 | (_,2) -> 2 | _ -> 3")(), 2);
  EXPECT_EQ(c().compileFn<int()>("match ([1,2],\"foo\") 2 with | _ 1 -> 1 | ([3,4],_) _ -> 2 | ([_,2],\"foo\") 2 -> 3 | _ _ -> 4")(), 3);

  EXPECT_EQ(c().compileFn<int()>("match (\"abc\", 2) with | (_, 2) -> 1 | (\"abc\", _) -> 2 | (_, 3) -> 3 | _ -> 4")(), 1);
  EXPECT_EQ(c().compileFn<int()>("match (\"abc\", 3) with | (_, 2) -> 1 | (\"abc\", _) -> 2 | (_, 3) -> 3 | _ -> 4")(), 2);
  EXPECT_EQ(c().compileFn<int()>("match (\"abd\", 3) with | (_, 2) -> 1 | (\"abc\", _) -> 2 | (_, 3) -> 3 | _ -> 4")(), 3);
  EXPECT_EQ(c().compileFn<int()>("match (\"abd\", 4) with | (_, 2) -> 1 | (\"abc\", _) -> 2 | (_, 3) -> 3 | _ -> 4")(), 4);
}

TEST(Matching, Variant) {
  EXPECT_EQ(c().compileFn<int()>("match (|0=(1,2,3)| :: (int*int*int)+int) with | |0=(x,y,z)| -> x+y+z | |1=y| -> y")(), 6);
  EXPECT_EQ(c().compileFn<int()>("match (|bob=3|::|bob:int,frank:[char]|) with | |frank=_| -> 9 | _ -> 2")(), 2);
  EXPECT_EQ(c().compileFn<int()>("match (|bob=3|::|bob:int,frank:[char]|) with | |bob=_| -> 9 | _ -> 2")(), 9);

  EXPECT_EQ(c().compileFn<int()>("match |foo=(\"abc\", 2)| with | |foo=(_, 2)| -> 1 | |foo=(\"abc\", _)| -> 2 | |foo=(_, 3)| -> 3 | |foo=_| -> 4")(), 1);
  EXPECT_EQ(c().compileFn<int()>("match |foo=(\"abc\", 3)| with | |foo=(_, 2)| -> 1 | |foo=(\"abc\", _)| -> 2 | |foo=(_, 3)| -> 3 | |foo=_| -> 4")(), 2);
  EXPECT_EQ(c().compileFn<int()>("match |foo=(\"abd\", 3)| with | |foo=(_, 2)| -> 1 | |foo=(\"abc\", _)| -> 2 | |foo=(_, 3)| -> 3 | |foo=_| -> 4")(), 3);
  EXPECT_EQ(c().compileFn<int()>("match |foo=(\"abd\", 4)| with | |foo=(_, 2)| -> 1 | |foo=(\"abc\", _)| -> 2 | |foo=(_, 3)| -> 3 | |foo=_| -> 4")(), 4);
}

TEST(Matching, Efficiency) {
  // make sure that we don't produce insane code for reasonable pattern-match expressions
  EXPECT_TRUE(c().machineCodeForExpr("(\\xs.match xs with | [1,2,3] -> 1 | [1,2,y] -> y | [] -> 9 | _ -> 10) :: [int] -> int").size() < 150);
}

TEST(Matching, Guards) {
  EXPECT_EQ(c().compileFn<int()>("match 1 2 3 with | 1 2 3 -> 0 | 1 2 y where y < 5 -> 1 | _ _ _ -> 2")(), 0);
  EXPECT_EQ(c().compileFn<int()>("match 1 2 4 with | 1 2 3 -> 0 | 1 2 y where y < 5 -> 1 | _ _ _ -> 2")(), 1);
  EXPECT_EQ(c().compileFn<int()>("match 1 2 5 with | 1 2 3 -> 0 | 1 2 y where y < 5 -> 1 | _ _ _ -> 2")(), 2);

  EXPECT_EQ(c().compileFn<int()>("match 1 2 5 with | 1 2 3 -> 0 | 1 x y where (x + y) == 7 -> 1 | _ _ _ -> 2")(), 1);
}

TEST(Matching, Regex) {
  // verify basic regex patterns
  EXPECT_EQ(c().compileFn<int()>("match \"foo\"  with | 'fo*'   -> 0 | _ -> 1")(), 0);
  EXPECT_EQ(c().compileFn<int()>("match \"foo\"  with | '(fo)*' -> 0 | _ -> 1")(), 1);
  EXPECT_EQ(c().compileFn<int()>("match \"fofo\" with | '(fo)*' -> 0 | _ -> 1")(), 0);

  // verify regex patterns within structures
  EXPECT_EQ((c().compileFn<int()>("match (\"jimmy\", \"chicken\") with | ('jimmy*', 'ab*') -> 0 | _ -> 1")()), 1);
  EXPECT_EQ((c().compileFn<int()>("match (\"jimmy\", \"chicken\") with | ('jimmy*', 'ab*') -> 0 | ('j*i*m*y*', 'chicken*') -> 42 | _ -> 1")()), 42);

  // verify various features of regex syntax
  EXPECT_EQ(c().compileFn<int()>("match \"aa\" with | 'a?a?' -> 0 | _ -> 1")(), 0);
  EXPECT_EQ(c().compileFn<int()>("match \"aa\" with | 'a?\\\\' -> 0 | _ -> 1")(), 1);
  EXPECT_EQ(c().compileFn<int()>("match \"a\\\\\" with | 'a?\\\\' -> 0 | _ -> 1")(), 0);
  EXPECT_EQ(c().compileFn<int()>("match \"a\\n\" with | 'a?\\\\' -> 0 | _ -> 1")(), 1);
  EXPECT_EQ(c().compileFn<int()>("match \"a\\n\" with | 'a?\\n' -> 0 | _ -> 1")(), 0);
  EXPECT_EQ(c().compileFn<int()>("match \"a\\n\" with | '[a-z]\\n' -> 0 | _ -> 1")(), 0);
  EXPECT_EQ(c().compileFn<int()>("match \"a\\n\" with | '[^a-z]\\n' -> 0 | _ -> 1")(), 1);
  EXPECT_EQ(c().compileFn<int()>("match \"0\\n\" with | '[^a-z]\\n' -> 0 | _ -> 1")(), 0);
  EXPECT_EQ(c().compileFn<int()>("match \"8675309\" with | '[0-9]+' -> 0 | _ -> 1")(), 0);

  // verify correct match/fallback logic with regexes and multiple columns
  EXPECT_EQ(c().compileFn<int()>("match \"ab\" 1 with | 'a(b|c)' 1 -> 1 | 'ab' 2 -> 2 | 'ac' 3 -> 3 | _ _ -> 4")(), 1);
  EXPECT_EQ(c().compileFn<int()>("match \"ab\" 2 with | 'a(b|c)' 1 -> 1 | 'ab' 2 -> 2 | 'ac' 3 -> 3 | _ _ -> 4")(), 2);
  EXPECT_EQ(c().compileFn<int()>("match \"ac\" 3 with | 'a(b|c)' 1 -> 1 | 'ab' 2 -> 2 | 'ac' 3 -> 3 | _ _ -> 4")(), 3);
  EXPECT_EQ(c().compileFn<int()>("match \"ab\" 3 with | 'a(b|c)' 1 -> 1 | 'ab' 2 -> 2 | 'ac' 3 -> 3 | _ _ -> 4")(), 4);
  EXPECT_EQ(c().compileFn<int()>("match \"foo\" 42 with | 'a(b|c)' 1 -> 1 | 'ab' 2 -> 2 | 'ac' 3 -> 3 | _ _ -> 4")(), 4);

  // verify unreachable row determination
  try {
    c().compileFn<int()>("match \"foo123ooo\" with | '123|foo.*' -> 0 | 'foo.*' -> 1 | _ -> -1");
    EXPECT_FALSE("failed to determine expected unreachable regex row");
  } catch (std::exception&) {
    EXPECT_TRUE(true);
  }

  // verify binding in regex matches
  EXPECT_EQ(makeStdString(c().compileFn<const array<char>*()>("match \"foobar\" with | 'f(?<os>o*)bar' -> os | _ -> \"???\"")()), "oo");
}

TEST(Matching, Support) {
  // we now have some support functions that could be used when compiling pattern match expressions and we need to make sure they're correct
  EXPECT_EQ(c().compileFn<long()>("bsearch([1,3],id,2)")(), 2);
  EXPECT_EQ(c().compileFn<long()>("bsearch([9,10],id,2)")(), 2);
  EXPECT_EQ(c().compileFn<long()>("bsearch([1,2,3,4],id,3)")(), 2);
}

TEST(Matching, Tests) {
  EXPECT_TRUE(c().compileFn<bool()>("\"8675309\" matches '[0-9]+'")());
  EXPECT_TRUE(c().compileFn<bool()>("(1,2) matches (1,2)")());
}

TEST(Matching, Functions) {
  // support irrefutable pattern matches in function heads
  EXPECT_EQ(c().compileFn<int()>("(\\(a,b) (c,d).a+b+c+d)((1, 2), (3, 4))")(), 10);
  EXPECT_EQ(c().compileFn<int()>("(\\{bob=a, frank=b} {chicken=c, jimmy=d}.a+b+c+d)({frank=1, bob=2}, {jimmy=3, chicken=4})")(), 10);

  // support refutable pattern matches in function heads
  EXPECT_TRUE(c().compileFn<bool()>("(\\[1,2,x].x+7)([1,2,3]) === |1=10|")());
  EXPECT_TRUE(c().compileFn<bool()>("(\\|1=x|.x+7)(just(3)) === |1=10|")());
}

TEST(Matching, Monadic) {
  // support irrefutable matching in monadic 'do' sequences
  EXPECT_EQ(c().compileFn<int()>("do { {x=x, y=y} = {x=1+2, y=3+4}; return x+y }")(), 10);
}

TEST(Matching, matchFromStringToBoolIsBool) {
  bool r = true;
  EXPECT_EQ(1, *(uint8_t*)(&r));

  r = c().compileFn<bool()>(
    "match \"1\" \"2\" \"3\" \"4\" with\n"
    "| \"1\" \"2\" \"3\" \"4\" -> true\n"
    "| \"1\" \"2\" \"3\" _     -> true\n"
    "| \"1\" \"2\" _ _         -> true\n"
    "| \"1\" _ _ _             -> true\n"
    "| _ _ _ _                 -> false"
  )();
  EXPECT_EQ(1, *(uint8_t*)(&r));
  EXPECT_TRUE(r);  
}

TEST(Matching, matchFromIntToBoolIsBool) {
  bool r = c().compileFn<bool()>(
    "match 1 2 3 4 with\n"
    "| 1 2 3 4 -> true\n"
    "| 1 2 3 _ -> true\n"
    "| 1 2 _ _ -> true\n"
    "| 1 _ _ _ -> true\n"
    "| _ _ _ _ -> false"
  );
  EXPECT_TRUE(1 == *(uint8_t*)(&r));
  EXPECT_TRUE(r);  
}

TEST(Matching, matchFromStringToIntIsCorrect) {
  int r = c().compileFn<int()>(
    "match \"1\" \"2\" \"3\" \"4\" with\n"
    "| \"1\" \"2\" \"3\" \"4\" -> 86\n"
    "| \"1\" \"2\" \"3\" _     -> 75\n"
    "| \"1\" \"2\" _ _         -> 30\n"
    "| \"1\" _ _ _             -> 9\n"
    "| _ _ _ _                 -> 0"
  )();
  EXPECT_EQ(86, *(uint32_t*)(&r));
  EXPECT_TRUE(r);  
}


