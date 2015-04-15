module Core;

use v6;

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

    '=' => -> $a, $b { make-bool($a.value eqv $b.value) },

    'empty?'    => list-op(-> $xs { make-bool(!$xs.Bool) }),
    'count'     => list-op(-> $xs { malInteger.new($xs.elems) }),
    'list'      => sub (*@items) { malList.new(@items) },
    'list?'     => isa(malList),

    ;

sub install-core(malEnv $env) is export {
    for %ns.kv -> $sym, $sub {
        $env.set($sym, malBuiltIn.new($sub));
    };
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

sub list-op($f) {
    return sub (malList $s) { $f($s.value) }
}
