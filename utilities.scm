(define num-gensym 0)

(define (gensym . opt)
  (let ((prefix (if (null? opt) "g" (car opt))))
    (set! num-gensym (+ num-gensym 1))
    (string->symbol
     (string-append prefix (number->string num-gensym) "_"))))

(define (ensure-string x)
  (cond ((null? x) "")
        ((symbol? x) (symbol->string x))
        ((number? x) (number->string x))
        ((pair? x) (string-append (ensure-string (car x))
                                  (ensure-string (cdr x))))
        (else x)))

(define (position x lst)
  (define (iter lst n)
    (cond ((null? lst) #f)
          ((eq? (car lst) x) n)
          (else (iter (cdr lst) (+ n 1)))))
  (iter lst 0))

(define (repeat-string n str)
  (define (iter n acc)
    (if (= n 0)
        acc
        (iter (- n 1) (string-append str acc))))
  (iter n ""))

(define (implode glue lst)
  (cond ((null? lst) lst)
        ((null? (cdr lst)) lst)
        (else (list* (car lst) glue (implode glue (cdr lst))))))

(define (copy-tree tree)
  (if (pair? tree)
      (cons (copy-tree (car tree)) (copy-tree (cdr tree)))
      tree))

(define builtin-list
  '(
    ;; 6.1. Equivalence predicates
    (eqv? 2 0)
    (eq? 2 0)
    (equal? 2 0)  ; library procedure
    ;; 6.2.5. Numerical operations
    (number? 1 0)
    (complex? 1 0)
    (real? 1 0)
    (rational? 1 0)
    (integer? 1 0)
    (exact? 1 0)
    (inexact? 1 0)
    (= 2 1)
    (< 2 1)
    (> 2 1)
    (<= 2 1)
    (>= 2 1)
    (zero? 1 0)  ; library procedure
    (positive? 1 0)  ; library procedure
    (negative? 1 0)  ; library procedure
    (odd? 1 0)  ; library procedure
    (even? 1 0)  ; library procedure
    (max? 1 1)  ; library procedure
    (min? 1 1)  ; library procedure
    (+ 0 1)
    (* 0 1)
    (- 1 1)  ; receive 1 or 2 arguments, and optional procedure receives any
    (/ 1 1)  ; receive 1 or 2 arguments, and optional procedure receives any
    (abs 1 0)  ; library procedure
    (quotient 2 0)
    (remainder 2 0)
    (modulo 2 0)
    (gcd 0 1)  ; library procedure
    (lcm 0 1)  ; library procedure
    (numerator 1 0)
    (denominator 1 0)
    (floor 1 0)
    (ceiling 1 0)
    (truncate 1 0)
    (round 1 0)
    (rationalize 2 0)  ; library procedure
    (exp 1 0)
    (log 1 0)
    (sin 1 0)
    (cos 1 0)
    (tan 1 0)
    (asin 1 0)
    (acos 1 0)
    (atan 1 1)  ; receive 1 or 2 arguments
    (sqrt 1 0)
    (expt 2 0)
    (make-rectangular 2 0)
    (make-polar 2 0)
    (real-part 1 0)
    (imag-part 1 0)
    (magnitude 1 0)
    (angle 1 0)
    (exact->inexact 1 0)
    (inexact->exact 1 0)
    ;; 6.2.6. Numerical input and output
    (number->string 1 1)  ; receive 1 or 2 arguments
    (string->number 1 1)  ; receive 1 or 2 arguments
    ;; 6.3.1. Booleans
    (not 1 0)  ; library procedure
    (boolean? 1 0)  ; library procedure
    ;; 6.3.2. Pairs and lists
    (pair? 1 0)
    (cons 2 0)
    (car 1 0)
    (cdr 1 0)
    (set-car! 2 0)
    (set-cdr! 2 0)
    (caar 1 0)  ; library procedure
    (cadr 1 0)  ; library procedure
    (cdar 1 0)  ; library procedure
    (cddr 1 0)  ; library procedure
    ;; cxxxr and cxxxxr are omitted
    (null? 1 0)  ; library procedure
    (list? 1 0)  ; library procedure
    (list 0 1)  ; library procedure
    (length 1 0)  ; library procedure
    (append 0 1)  ; library procedure
    (reverse 1)  ; library procedure
    (list-tail 2 0)  ; library procedure
    (list-ref 2 0)  ; library procedure
    (memq 2 0)  ; library procedure
    (memv 2 0)  ; library procedure
    (member 2 0)  ; library procedure
    (assq 2 0)  ; library procedure
    (assv 2 0)  ; library procedure
    (assoc 2 0)  ; library procedure
    ;; 6.3.3. Symbols
    (symbol? 1 0)
    (symbol->string 1 0)
    (string->symbol 1 0)
    ;; 6.3.4. Characters
    (char? 1 0)
    (char=? 2 0)
    (char<? 2 0)
    (char>? 2 0)
    (char<=? 2 0)
    (char>=? 2 0)
    (char-ci=? 2 0)  ; library procedure
    (char-ci=<? 2 0)  ; library procedure
    (char-ci=>? 2 0)  ; library procedure
    (char-ci=<=? 2 0)  ; library procedure
    (char-ci=>=? 2 0)  ; library procedure
    (char-alphabetic? 1 0)  ; library procedure
    (char-numeric? 1 0)  ; library procedure
    (char-whitespace? 1 0)  ; library procedure
    (char-upper-case? 1 0)  ; library procedure
    (char-lower-case? 1 0)  ; library procedure
    (char->integer 1 0)
    (integer->char 1 0)
    (char-upcase 1 0)  ; library procedure
    (char-downcase 1 0)  ; library procedure
    ;; 6.3.5. Strings
    (string? 1 0)
    (make-string 1 0)
    (string 0 1)  ; library procedure
    (string-length 1 0)
    (string-ref 2 0)
    (string-set! 3 0)
    (string=? 2 0)  ; library procedure
    (string-ci=? 2 0)  ; library procedure
    (string<? 2 0)  ; library procedure
    (string>? 2 0)  ; library procedure
    (string<=? 2 0)  ; library procedure
    (string>=? 2 0)  ; library procedure
    (string-ci<? 2 0)  ; library procedure
    (string-ci>? 2 0)  ; library procedure
    (string-ci<=? 2 0)  ; library procedure
    (string-ci>=? 2 0)  ; library procedure
    (substring 3 0)  ; library procedure
    (string-append 0 1)  ; library procedure
    (string->list 1 0)  ; library procedure
    (list->string 1 0)  ; library procedure
    (string-copy 1 0)  ; library procedure
    (string-fill! 2 0)  ; library procedure
    ;; 6.3.6. Vectors
    (vector? 1 0)
    (make-vector 1 1)  ; receive 1 or 2 arguments
    (vector 0 1)  ; library procedure
    (vector-length 1 0)
    (vector-ref 2 0)
    (vector-set! 3 0)
    (vector->list 1 0)  ; library procedure
    (list->vector 1 0)  ; library procedure
    (vector-fill! 2 0)  ; library procedure
    ;; 6.4. Control features
    (procedure? 1 0)
    (apply 2 1)
    (map 2 1)  ; library procedure
    (for-each 2 1)  ; library procedure
    (force 1 0)  ; library procedure
    (call-with-current-continuation 1 0)
    (values 0 1)
    (call-with-values 2 0)
    (dynamic-wind 3 0)
    ;; 6.5. Eval
    (eval 2 0)
    (scheme-report-environment 1 0)
    (null-environment 1 0)
    (interaction-environment 0 0)  ; optional procedure
    ;; 6.6.1. Ports
    (call-with-input-file 2 0)  ; library procedure
    (call-with-output-file 2 0)  ; library procedure
    (input-port? 1 0)
    (output-port? 1 0)
    (current-input-port 0 0)
    (current-output-port 0 0)
    (with-input-from-file 2 0)  ; optional procedure
    (with-output-to-file 2 0)  ; optional procedure
    (open-input-file 1 0)
    (open-output-file 1 0)
    (close-input-port 1 0)
    (close-output-port 1 0)
    ;; 6.6.2. Input
    (read 0 1)  ; receive 0 or 1 arguments
    (read-char 0 1)  ; receive 0 or 1 arguments
    (peek-char 0 1)  ; receive 0 or 1 arguments
    (eof-object? 1 0)
    (char-ready? 0 1)  ; receive 0 or 1 arguments
    ;; 6.6.3. Output
    (write 1 1)  ; library procedure, receive 1 or 2 arguments
    (display 1 1)  ; library procedure, receive 1 or 2 arguments
    (newline 0 1)  ; library procedure, receive 0 or 1 arguments
    (write-char 1 1)  ; receive 1 or 2 arguments
    ;; 6.6.4. System interface
    (load 1 0)  ; optional procedure
    (transcript-on 1 0)  ; optional procedure
    (transcript-off 0 0)  ; optional procedure
    ))
