module Core;

use v6;

use Printer;
use Reader;
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

    '=' => sub ($a, $b) { make-bool(is-eq($a, $b)) },

    'cons' => sub ($first, malSequence $rest) {
        malList.new($first, $rest.value.list)
    },
    'concat' => sub (*@seqs) {
        my @all = @seqs.for: -> malSequence $seq { $seq.value.list };
        malList.new(@all);
    },

    'count'  => sub ($x) {
        return malInteger.new(0) if $x ~~ malNil;
        for $x -> malSequence $xs {
            return malInteger.new($xs.value.elems);
        }
    },
    'empty?'  => sub (malSequence $xs) { make-bool(!$xs.value.Bool) },
    'first'   => sub (malSequence $xs) {
        $xs.value.elems > 0 ?? $xs.value[0] !! malNil
    },
    'list'    => sub (*@xs) { malList.new(@xs) },
    'list?'   => isa(malList),
    'nth'     => sub (malSequence $s, malInteger $index) {
        my $i = $index.value;
        die RuntimeError.new("Index $i out of range")
            if ($i < 0) || ($i >= $s.value.elems);
        return $s.value[$i];
    },

    'pr-str'  => sub (*@xs) { str-join(@xs, True,  " ", False) },
    'str'     => sub (*@xs) { str-join(@xs, False, "",  False) },
    'prn'     => sub (*@xs) { str-join(@xs, True,  " ", True)  },
    'println' => sub (*@xs) { str-join(@xs, False, " ", True) },

    'read-string' => sub (malString $s) { read-str($s.value) },
    'rest'    => sub (malSequence $xs) { malList.new($xs.value[1..*]) },
    'slurp'   => sub (malString $s) {
        my $fn = $s.value;
        die RuntimeError.new("File \"$fn\" not found") unless $fn.IO ~~ :e;
        return malString.new(slurp $fn);
    },
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

sub str-join(@args, Bool $readably, Str $sep, Bool $print) {
    my $str = @args.map({ pr-str($_, $readably) }).join($sep);
    if $print {
        say $str;
        return malNil;
    }
    return malString.new($str);
}
