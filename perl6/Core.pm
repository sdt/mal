unit module Core;

use v6;

use Printer;
use Reader;
use ReadLine;
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
    'assoc' => sub (malHash $hash, *@pairs) {
        make-hash(( $hash.value.kv, @pairs ).list);
    },
    'atom' => sub ($value) { malAtom.new($value) },
    'atom?' => isa(malAtom),
    'cons' => sub ($first, malSequence $rest) {
        malList.new($first, $rest.value.list)
    },
    'concat' => sub (*@seqs) {
        my @all = @seqs.for: -> malSequence $seq { $seq.value.list };
        malList.new(@all);
    },
    'conj' => sub (malSequence $seq, *@args) {
        given $seq {
            when malList {
                return malList.new(@( @args.reverse.list, $seq.value.list ));
            }
            when malValue {
                return malVector.new([ $seq.value.list, @args.list ]);
            }
        }
    },
    'contains?' => sub (malHash $hash, $key) {
        malBoolean.new($hash.value{make-hash-key($key)}:exists);
    },
    'count'  => sub ($x) {
        return malInteger.new(0) if $x ~~ malNil;
        for $x -> malSequence $xs {
            return malInteger.new($xs.value.elems);
        }
    },
    'deref' => sub (malAtom $atom) { $atom.value },
    'dissoc' => sub (malHash $hash, *@keys) {
        # Create a set of the keys to be removed.
        my $to-remove = @keys.map({ make-hash-key($_) }).Set;

        # Create a new hash by filtering out the unwanted pairs.
        my %filtered = $hash.value.pairs.grep: { $_.key !(elem) $to-remove };

        malHash.new(%filtered);
    },
    'empty?'  => sub (malSequence $xs) { make-bool(!$xs.value.Bool) },
    'eval'    => sub (malValue $ast) { $eval($ast) },
    'false?'  => isa(malFalse),
    'first'   => sub (malSequence $xs) {
        $xs.value.elems > 0 ?? $xs.value[0] !! malNil
    },
    'get' => sub ($hashmap, $key) {
        return $hashmap if $hashmap ~~ malNil;
        for $hashmap -> malHash $hash {
            return $hash.value{make-hash-key($key)} // malNil;
        }
    },
    'hash-map' => sub (*@pairs) { make-hash(@pairs) },
    'keys' => sub (malHash $hash) {
        malList.new($hash.value.keys.map({
            #TODO: urgh...
            $_.substr(0,1) eq ':'
                ?? malKeyword.new($_)
                !! malString.new(unescape-string($_))
        }));
    },
    'keyword' => sub (malString $s) { malKeyword.new(':' ~ $s.value) },
    'keyword?' => isa(malKeyword),
    'list'    => sub (*@xs) { malList.new(@xs) },
    'list?'   => isa(malList),
    'map'     => sub ($f, malSequence $seq) {
        # Something weird happens to exceptions with this one.
        #   malList.new($seq.value.map({ $apply($f, [ $_ ]) }))
        # Going via @mapped seems to fix it :/
        my @mapped = $seq.value.map({ $apply($f, [ $_ ]) });
        malList.new(@mapped);
    },
    'map?'    => isa(malHash),
    'meta'    => sub ($x) { $x.meta // malNil },
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

    'readline' => sub (malString $prompt) {
        my $line = read-line($prompt.value);
        return $line.defined ?? malString.new($line) !! malNil;
    },
    'read-string' => sub (malString $s) { read-str($s.value) },
    'reset!' => sub (malAtom $atom, $new) { $atom.value = $new },
    'rest'    => sub (malSequence $xs) { malList.new($xs.value[1..*]) },
    'slurp'   => sub (malString $s) {
        my $fn = $s.value;
        die RuntimeError.new("File \"$fn\" not found") unless $fn.IO ~~ :e;
        return malString.new(slurp $fn);
    },
    'sequential?' => isa(malSequence),
    'swap!' => sub (malAtom $atom, $f, *@args) {
        @args.unshift($atom.value);
        $atom.value = $apply($f, @args);
    },
    'symbol' => sub (malString $s) { malSymbol.new($s.value) },
    'symbol?' => isa(malSymbol),
    'throw' => sub (malValue $exception) { die $exception },
    'true?' => isa(malTrue),
    'vals' => sub (malHash $hash) { malList.new($hash.value.values) },
    'vector' => sub (*@xs) { malVector.new(@xs) },
    'vector?' => isa(malVector),
    'with-meta' => sub ($value, $meta) {
        return $value.with-meta($meta);
    },
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

    given $lhs {
        when malHash {
            return False unless $lhs.value.elems == $rhs.value.elems;

            my @lhs-keys = $lhs.value.keys.sort;
            my @rhs-keys = $rhs.value.keys.sort;
            return False unless @lhs-keys eqv @rhs-keys;

            return list-eq($lhs.value{@lhs-keys}, $rhs.value{@rhs-keys});
        }
        when malAtom {
            return is-eq($lhs.value, $rhs.value);
        }
        when malBoolean {
            # Can't use smartmatch for booleans. Hooray.
            return $lhs.value == $rhs.value;
        }
        default {
            # Use smartmatch for the rest.
            return $lhs.value ~~ $rhs.value;
        }
    }
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
