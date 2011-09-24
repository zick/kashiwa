(load "./utilities.scm")

(define num-arguments-limit 4)

(define (has-optional? args)
  (cond ((null? args) #f)
        ((pair? args) (has-optional? (cdr args)))
        (else #t)))

(define (make-function)
  ;; name, arguments, local variables, and body (inverse order)
  (list #f '() '() '()))
(define (function-name fun)
  (car fun))
(define (set-function-name! fun name)   ; 'name' is a symbol
  (set-car! fun name))
(define (function-args fun)
  (cadr fun))
(define (set-function-args! fun args)  ; 'args' is a list of 'arg'
  (set-car! (cdr fun) args))
(define (push-function-args! arg fun)  ; 'arg' is a pair of type and name
  (set-function-args!
   fun
   (cons arg (function-args fun))))
(define (function-vars fun)
  (caddr fun))
(define (set-function-vars! fun vars)  ; 'vars' is a list of 'var'
  (set-car! (cddr fun) vars))
(define (push-function-vars! var fun)  ; 'var' is a pair of type and name
  (set-function-vars!
   fun
   (cons var (function-vars fun))))
(define (function-body fun)
  (cadddr fun))
(define (set-function-body! fun body)
  (set-car! (cdddr fun) body))
(define (push-function-body! b fun)
  (set-function-body!
   fun
   (cons b (function-body fun))))

(define (translate-prototype fun . ret-type)
  (string-append
   (if (null? ret-type) "static void" (ensure-string (car ret-type)))
   " "
   (ensure-string (function-name fun))
   "("
   (apply
    string-append
    (implode
     ", "
     (map (lambda (p)
            (string-append (ensure-string (car p))
                           " "
                           (ensure-string (cdr p))))
          (let ((args (function-args fun)))  ; args can be ().
            (if (> (- (length args) 1) num-arguments-limit)
                (list (car args) (cadr args))
                args)))))
   ")"))

(define (translate-local-vars vars indent)
  (apply
   string-append
   (map (lambda (p)
          (string-append (repeat-string indent " ")
                         (ensure-string (car p))
                         " "
                         (ensure-string (cdr p))
                         ";" (string #\Newline)))
        vars)))

(define (translate-argument arg)
  (cond ((and (pair? arg) (eq? (car arg) 'int2fixnum))
         (string-append "INT2FIXNUM(" (ensure-string (cadr arg)) ")"))
        ((boolean? arg) (if arg "sharpt" "sharpf"))
        (else (ensure-string arg))))

(define (translate-builtin-call fn args)
  (string-append
   "builtin_"
   (lisp-name-to-c-name (ensure-string fn))
   "((void*)0, (cont_t*)"
   (apply
    string-append
    (implode ", " (map translate-argument args)))
   ");" (string #\Newline)))

(define (translate-cont-call clos args . safe)
  (string-append
   (if (null? safe) "RAW_CONTINUE" "CONTINUE")
   (ensure-string (length args))
   "("
   (ensure-string clos)
   ", "
   (apply
    string-append
    (implode ", " (map translate-argument args)))
   ");" (string #\Newline)))

(define (translate-safe-cont-call clos args)
  (translate-cont-call clos args #t))

(define (translate-if test then alt indent)
  (string-append
   "if (" (ensure-string test) ") {" (string #\Newline)
   (repeat-string (+ indent 2) " ")
   (translate-sentence then (+ indent 2))
   (repeat-string indent " ")
   "} else {" (string #\Newline)
   (repeat-string (+ indent 2) " ")
   (translate-sentence alt (+ indent 2))
   (repeat-string indent " ")
   "}" (string #\Newline)))

(define (translate-sentence s indent)
  (cond ((not (pair? s)) (error "Translation error"))
        ((pair? (car s))
         (apply string-append
                (implode (repeat-string indent " ")
                         (translate-sentence-list s indent))))
        ((eq? (car s) 'builtin-call)
         (translate-builtin-call (cadr s) (caddr s)))
        ((eq? (car s) 'proc-call)
         (translate-safe-cont-call (cadr s) (caddr s)))
        ((eq? (car s) 'cont-call)
         (translate-cont-call (cadr s) (caddr s)))
        ((eq? (car s) 'safe-cont-call)
         (translate-safe-cont-call (cadr s) (caddr s)))
        ((eq? (car s) 'if)
         (translate-if (cadr s) (caddr s) (cadddr s) indent))
        (else (string-append (ensure-string s) ";" (string #\Newline)))))

(define (translate-sentence-list slist indent)
  (map (lambda (x) (translate-sentence x indent)) slist))

(define (translate-body body indent)
  (apply
   string-append
   (cons
    (repeat-string indent " ")
    (implode
     (repeat-string indent " ")
     (translate-sentence-list body indent)))))

(define (translate-to-c fun . ret-type)
  (string-append
   (apply translate-prototype fun ret-type)
   " {" (string #\Newline)
   (translate-local-vars (function-vars fun) 2)
   (translate-body (reverse (function-body fun)) 2)
   "}" (string #\Newline)
   ))

(define lambda-names '())
(define (add-lambda-name exp name)
  (set! lambda-names
        (cons (cons exp name) lambda-names)))
(define (find-lambda-name exp)
  (let ((ret (assq exp lambda-names)))
    (and ret (cdr ret))))
(define compiled-results '())
(define (add-compiled-function name fun)
  (set! compiled-results
        (cons (cons name fun) compiled-results)))
(define (find-compiled-function name)
  (let ((ret (assq name compiled-results)))
    (and ret (cdr ret))))
(define (init-compile)
  (set! lambda-names '())
  (set! compiled-results '()))

(define (has-lambda? exp)
  (define (check-arguments args)
    (cond ((null? args) #f)
          ((has-lambda? (car args)) #t)
          (else (check-arguments (cdr args)))))
  (cond ((not (pair? exp)) #f)
        ((eq? (car exp) 'lambda) #t)
        ((has-lambda? (car exp)) #t)
        (else (check-arguments (cdr exp)))))


(define (lisp-name-to-c-name str)
  (do ((i 0 (+ i 1))
       (acc '()))
      ((= i (string-length str)) (apply string (reverse acc)))
    (let ((c (string-ref str i)))
      (cond ((eq? c #\-) (set! acc (cons #\_ acc)))
            ((eq? c #\?) (set! acc (cons #\p acc)))
            ((eq? c #\!) (set! acc (list* #\n #\a #\b acc)))
            ((eq? c #\=) (set! acc (list* #\l #\q #\e acc)))
            ((eq? c #\+) (set! acc (list* #\s #\u #\l #\p acc)))
            ((eq? c #\*) (set! acc (list* #\r #\a #\t #\s acc)))
            ((eq? c #\/) (set! acc (list* #\a #\l #\s acc)))
            (else (set! acc (cons c acc)))))))

(define (lookup-var var env)
  (define (iter env level)
    (cond ((null? env) #f)
          ((member var (car env))
           (list level (position var (car env))))
          (else (iter (cdr env) (+ level 1)))))
  (iter env 0))

(define (gen-lookup-var-code var link pos fun)
  (if (= link 0)
      ;; If the number of arguments is greater than num-arguments-limit,
      ;; function-args return the list (env args_env original-args ...).
      ;; args_env->vars[i] hold the value of argument.
      ;; original-args is used at only compile time.
      (if (> (- (length (function-args fun)) 1) num-arguments-limit)
          (list (cdadr (function-args fun)) "->vars[" pos "]")
          var)
      (list
       (cdar (function-args fun))
       (repeat-string (- link 1) "->link")
       "->vars[" pos "]")))

(define (gen-lookup-var var env fun)
  (let* ((loc (lookup-var var env))
         (link (and loc (car loc)))
         (pos (and loc (cadr loc))))
    (if loc
        (gen-lookup-var-code var link pos fun)
        var)))  ; global variable

(define (gen-literal-lambda-code exp env fun)
  (gen-lambda-code exp env)
  (let ((clos (gensym "clos"))
        (cenv
         (if (> (- (length (function-args fun)) 1) num-arguments-limit)
          (cdadr (function-args fun))
          (cdr (list-ref (function-vars fun)
                         (- (length (function-vars fun)) 1)))))
        (name (find-lambda-name exp))
        (len (length (cadr exp))))
    (push-function-vars! (cons "cont_t" clos) fun)
    (push-function-body!
     (list
      (list clos ".tag = TAG_CONT")
      (list clos ".env = " cenv)
      (list clos ".fn = (function1_t)" name)
      (list clos ".num_required_args = "
            (if (> len num-arguments-limit) 1 len))
      (list clos ".optional_args = "
            (if (and (has-optional? (cadr exp)) (<= len num-arguments-limit))
                1 0)))
     fun)
    (list "&" clos)))

(define (gen-literal-code exp env fun)
  (cond ((number? exp)
         (list 'int2fixnum exp))
        ((symbol? exp)
         (gen-lookup-var exp env fun))
        ((boolean? exp) exp)
        ((and (pair? exp) (eq? (car exp) 'quote))
         (gen-quote-code exp env fun))
        ((and (pair? exp) (eq? (car exp) 'set!))
         (gen-set!-code exp env fun))
        ((and (pair? exp) (eq? (car exp) 'lambda))
         (gen-literal-lambda-code exp env fun))))

(define (gen-quote-code exp env fun)
  (cond ((number? (cadr exp))
         (list 'int2fixnum (cadr exp)))
        ((symbol? (cadr exp))
         (let ((sym (gensym "sym")))
           (push-function-vars! (cons "static lobject" sym) fun)
           (push-function-body!
            (list 'if (list sym " == 0")
                  (list
                   (list sym " = intern(\"" (cadr exp) "\")")
                   (list "add_symbol_rootset(&" sym ")"))
                  (list ""))
            fun)
           sym))
        (else
         (error "Not implemented"))))

(define (gen-if-code exp env fun)
  (list 'if
        (list "BOOLTEST(" (gen-lookup-var (cadr exp) env fun) ")")
        (gen-apply-code (caddr exp) env fun)
        (gen-apply-code (cadddr exp) env fun)))

(define (gen-builtin-application exp env fun)
  (list 'builtin-call (car exp)
        (map (lambda (x) (gen-literal-code x env fun)) (cdr exp))))

(define (gen-application-many-args args fun)
  (let ((args-env (gensym "args_env")))
    (push-function-vars! (cons "env_t*" args-env) fun)
    (push-function-body!
     (list
      (list args-env " = alloca(sizeof(env_t) + sizeof(lobject) * "
            (- (length args) 1) ")")
      (list args-env "->tag = TAG_ENV")
      (list args-env "->num = " (length args))
      (list args-env "->link = NULL"))
     fun)
    (do ((args args (cdr args))
         (i 0 (+ i 1))
         (acc '()))
        ((null? args) #t)
      (push-function-body!
       (list args-env "->vars[" i "] = (lobject)"
             (translate-argument (car args)))
       fun))))

(define (gen-user-proc-application exp env fun)
  (let ((args (map (lambda (x) (gen-literal-code x env fun)) (cdr exp))))
    (if (> (length args) num-arguments-limit)
        (begin (gen-application-many-args args fun)
               (set! args (list (cdar (function-vars fun))))))
    (list 'proc-call (car exp) args)))

(define (gen-user-closure-application name args env fun)
  (let ((clos (gensym "clos"))
        (cenv
         (if (> (- (length (function-args fun)) 1) num-arguments-limit)
             (cdadr (function-args gun))
             (cdr (list-ref (function-vars fun)
                            (- (length (function-vars fun)) 1))))))
    (push-function-vars! (cons "cont_t" clos) fun)
    (list
     (list clos ".tag = TAG_CONT")
     (list clos ".env = " cenv)
     (list clos ".fn = (function1_t)" name)
     (let ((args (map (lambda (x) (gen-literal-code x env fun)) args)))
       (if (> (length args) num-arguments-limit)
           (begin (gen-application-many-args args fun)
                  (set! args (list (cdar (function-vars fun))))))
       (list 'cont-call (list "&" clos) args)))))

(define (gen-continuation-code exp env fun)
  (list 'safe-cont-call
        (gen-lookup-var (car exp) env fun)
        (let ((args (map (lambda (x) (gen-literal-code x env fun)) (cdr exp))))
          (if (> (length args) num-arguments-limit)
              (begin (gen-application-many-args args fun)
                     (set! args (list (cdar (function-vars fun))))))
          args)))

(define (gen-set!-assign-code var val fun)
  (push-function-body!
    (if (symbol? var)  ; global variable
        (list
         var
         " = (lobject)"
         (translate-argument val))
        (list
         "SET_WITH_BARRIER("
         (ensure-string (reverse (cdddr (reverse var))))  ; environment
         ", "
         (ensure-string var)
         ", "
         (translate-argument val)
         ")"))
    fun))

(define (gen-set!-code exp env fun)
  (gen-set!-assign-code
        (if (lookup-var (cadr exp) env)
            (gen-lookup-var (cadr exp) env fun)
            (cadr exp))  ; global variable
        (gen-literal-code (caddr exp) env fun)
        fun)
  (list 'int2fixnum 0))  ; undefined value

(define (gen-apply-code exp env fun)
  (cond ((not (pair? exp)) (error "Compile error"))
        ((eq? (car exp) 'if)
         (gen-if-code exp env fun))
        ((pair? (car exp))  ; lambda-exp
         (gen-lambda-code (car exp) env)
         (gen-user-closure-application
          (find-lambda-name (car exp))
          (cdr exp)
          env
          fun))
        ((lookup-var (car exp) env)
         (gen-continuation-code exp env fun))
        ((member (car exp) builtin-list)  ; builtin function
         (gen-builtin-application exp env fun))
        (else
         (gen-user-proc-application exp env fun))))

(define (gen-current-env-for-many-args fun args)
  (let ((penv (cdar (function-args fun)))
        (args (cdadr (function-args fun)))
        (len (length (cddr (function-args fun)))))
    (list
     (list 'if (list args "->tag != TAG_ENV || " args "->num"
                     (if (has-optional? args) " < " " != ") len)
           (list (list "fprintf(stderr, \"Wrong number of arguments.\\n\")")
                 (list "exit(1)"))
           (list ""))
     (list args "->link = " penv))))

(define (gen-current-env fun)
  ;; assume that the first variable is cenv and the first argument is penv.
  (let ((ret '())
        (cenv (cdar (function-vars fun)))
        (penv (cdar (function-args fun))))
    (set!
     ret
     (cons (list cenv " = (env_t*)alloca(sizeof(env_t) + sizeof(lobject) * "
                 (- (length (function-args fun)) 2) ")")
           ret))
    (set! ret (cons (list cenv "->tag = TAG_ENV") ret))
    (set! ret (cons (list cenv "->num = "
                          (- (length (function-args fun)) 1)) ret))
    (set! ret (cons (list cenv "->link = " penv) ret))
    (do ((i 0 (+ i 1))
         (args (cdr (function-args fun)) (cdr args)))
        ((null? args) #t)
      (set! ret
            (cons (list cenv "->vars[" i "] = (lobject)" (cdar args)) ret)))
    (reverse ret)))

(define (gen-check-stack-code fun)
  (let ((name (function-name fun))
        (args (map cdr (function-args fun))))
    (if (> (- (length args) 1) num-arguments-limit)
        (set! args (list (car args) (cadr args))))
    (append
     (list* "check_stack(" name ", " (car args) ", " (length (cdr args)) ", "
            (implode ", " (cdr args)))
     '(")"))))

(define (gen-lambda-code exp env)
  (if (find-lambda-name exp)
      'already-compiled
      (let ((fun (make-function))
            (args (cadr exp))
            (body (caddr exp)))
        (set-function-name! fun (gensym "fun"))
        (set-function-args!
         fun
         (map (lambda (x) (cons 'lobject x)) args))
        (if (> (length args) num-arguments-limit)
            (push-function-args! (cons "env_t*" (gensym "args")) fun))
        (push-function-args! (cons "env_t*" (gensym "penv")) fun)
        (push-function-body! (gen-check-stack-code fun) fun)
        (if (> (length args) num-arguments-limit)
            (push-function-body! (gen-current-env-for-many-args fun args) fun)
            (if (has-lambda? body)
                (begin
                  (push-function-vars! (cons "env_t*" (gensym "cenv")) fun)
                  (push-function-body! (gen-current-env fun) fun))))
        (add-lambda-name exp (function-name fun))
        (push-function-body!
         (gen-apply-code body (cons args env) fun)
         fun)
        (add-compiled-function (function-name fun) fun)
        fun)))

(define (gen exp)
  ;; assume that exp is the lambda expression.
  (if (and (pair? exp) (eq? (car exp) 'lambda))
      (gen-lambda-code exp '())
      (error "Compile error")))

(define (show-result)
  (for-each
   (lambda (x)
     (display (string-append (translate-prototype (cdr x)) ";")) (newline))
   compiled-results)
  (for-each
   (lambda (x)
     (display (translate-to-c (cdr x)))(newline))
   compiled-results))