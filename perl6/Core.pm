module Core;

use v6;

use Printer;
use Reader;
use Types;

# apply and eval get passed in to avoid circular dependency problems.
sub install-core(malEnv $env, :$apply, :$eval) is export {
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

    'apply' => sub ($f, *@args) {
        my malSequence $last = @args.pop;
        @args.push($last.value.list);
        $apply($f, @args);
    },
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
    'eval'    => sub (malValue $ast) { $eval($ast) },
    'false?'  => isa(malFalse),
    'first'   => sub (malSequence $xs) {
        $xs.value.elems > 0 ?? $xs.value[0] !! malNil
    },
    'keyword' => sub (malString $s) { malKeyword.new(':' ~ $s.value) },
    'keyword?' => isa(malKeyword),
    'list'    => sub (*@xs) { malList.new(@xs) },
    'list?'   => isa(malList),
    'map'     => sub ($f, malSequence $seq) {
        malList.new($seq.value.map({ $apply($f, [ $_ ]) }))
    },
    'map?'    => isa(malHash),
    'nil?'    => isa(malNil),
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
    'sequential?' => isa(malSequence),
    'symbol' => sub (malString $s) { malSymbol.new($s.value) },
    'symbol?' => isa(malSymbol),
    'throw' => sub (malValue $exception) { die $exception },
    'true?' => isa(malTrue),
    'vector' => sub (*@xs) { malVector.new(@xs) },
    'vector?' => isa(malValue),
    ;

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
        my $value = $f($a.value, $b.value).Int;
        if $value ~~ Failure {
            # This causes the divide-by-zero Failure to throw.
            # Presumably this is some kind of feature...
        }
        return malInteger.new($value.Int);
    };
}

sub int-rel($f) {
    return sub (malInteger $a, malInteger $b) {
        return make-bool($f($a.value, $b.value));
    };
}

sub isa($type) {
    return sub ($x) { make-bool($x ~~ $type) };
}

sub str-join(@args, Bool $readably, Str $sep, Bool $print) {
    my $str = @args.map({ pr-str($_, $readably) }).join($sep);
    if $print {
        say $str;
        return malNil;
    }
    return malString.new($str);
}
