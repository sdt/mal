module Core;

use v6;

use Types;

my %ns =
    '+' => int-op(* + *),
    '-' => int-op(* - *),
    '*' => int-op(* * *),
    '/' => int-op(* / *),

    'list'  => sub (*@items) { malList.new(@items) },
    'list?' => isa(malList),

    ;

sub install-core(malEnv $env) is export {
    for %ns.kv -> $sym, $sub {
        $env.set($sym, malBuiltIn.new($sub));
    };
}

sub int-op($native-func) {
    return sub (malInteger $a, malInteger $b) {
        my $value = $native-func($a.value, $b.value);
        return malInteger.new($value.Int);
    };
}

sub isa($type) {
    return sub (malValue $x) {
        return $x ~~ $type ?? malTrue !! malFalse;
    };
}
