module Core;

use v6;

use Printer;
use Types;

my %ns =
    '+' => int-op(* + *),
    '-' => int-op(* - *),
    '*' => int-op(* * *),
    '/' => int-op(* / *),

    '<'  => int-rel(* <  *),
    '<=' => int-rel(* <= *),
    '>'  => int-rel(* >  *),
    '>=' => int-rel(* >= *),

    '=' => -> $a, $b { make-bool(is-eq($a, $b)) },

    'empty?'    => seq-op(-> $xs { make-bool(!$xs.Bool) }),
    'count'     => seq-op(-> $xs { malInteger.new($xs.elems) }),
    'list'      => sub (*@items) { malList.new(@items) },
    'list?'     => isa(malList),

    'pr-str'  => sub (*@values) { str-join(@values, True, " ", False) },
    'str'     => sub (*@values) { str-join(@values, False, "", False) },
    'prn'     => sub (*@values) { str-join(@values, True, " ", True)  },
    'println' => sub (*@values) { str-join(@values, False, " ", True) },
    ;

sub install-core(malEnv $env) is export {
    for %ns.kv -> $sym, $sub {
        $env.set($sym, malBuiltIn.new($sub));
    };
}

sub is-eq(malValue $lhs, malValue $rhs) {
    if $lhs ~~ malSequence && $rhs ~~ malSequence {
        return list-eq($lhs.value.list, $rhs.value.list);
    }

    return False unless $lhs.WHAT ~~ $rhs.WHAT;

    if $lhs ~~ malHash {
        return False unless $lhs.value.elems == $rhs.value.elems;

        my @lhs-keys = $lhs.value.keys.sort;
        my @rhs-keys = $rhs.value.keys.sort;
        return False unless @lhs-keys eqv @rhs-keys;

        return list-eq($lhs.value{@lhs-keys}, $rhs.value{@rhs-keys});
    }

    return $lhs.value ~~ $rhs.value;
}

sub list-eq(@lhs, @rhs) {
    return False unless @lhs.elems == @rhs.elems;

    for zip(@lhs; @rhs) -> $a, $b {
        return False unless is-eq($a, $b);
    }
    return True;
}

sub int-op($f) {
    return sub (malInteger $a, malInteger $b) {
        my $value = $f($a.value, $b.value);
        return malInteger.new($value.Int);
    };
}

sub int-rel($f) {
    return sub (malInteger $a, malInteger $b) {
        return make-bool($f($a.value, $b.value));
    };
}

sub isa($type) {
    return sub (malValue $x) { make-bool($x ~~ $type) };
}

sub seq-op($f) {
    return sub (malSequence $s) { $f($s.value) }
}

sub str-join(@args, Bool $readably, Str $sep, Bool $print) {
    my $str = @args.map({ pr-str($_, $readably) }).join($sep);
    if $print {
        say $str;
        return malNil;
    }
    return malString.new($str);
}
