(define kitt-part
  (composite rectangle
    (prop 'width 64)
    (prop 'height 64)
    (prop 'phase 1)
    (prop 'n 1)
    (prop 'color (bound '(this phase this n) (lambda (phase n)
      (list (floor (/ (* 255 phase) n)) 0 0))))))
      
(define (kitt num-parts)
  (define (kitt-phase i cur)
    (- num-parts (abs (- i cur))))

  (define (kitt-bound i)
    (bound '(parent phase) (lambda (cur)
      (kitt-phase i cur))))

  (define (make-kitt-part i)
    (kitt-part
      (prop 'phase (kitt-bound (+ i 1)))
      (prop 'n num-parts)))

  (composite row
    (prop 'phase 1)
    (prop 'direction 1)
    (arg-list (map make-kitt-part (iota num-parts)))
    (timer
      (prop 'interval 0.1)
      (prop 'running #t)
      (prop 'repeat -1)
      (method 'on-timeout (lambda (c)
        (let ((next-phase (+ (get-prop (parent) 'phase)
                             (get-prop (parent) 'direction))))
          (cond ((> next-phase num-parts)
                 (begin
                   (set-prop! (parent) 'phase (- num-parts 1))
                   (set-prop! (parent) 'direction -1)))
                ((< next-phase 1)
                 (begin
                   (set-prop! (parent) 'phase 2)
                   (set-prop! (parent) 'direction 1)))
                (else (set-prop! (parent) 'phase next-phase)))))))))

(define kitt-7 (kitt 7))
(define kitt-9 (kitt 9))

(create-elements!
  (column (prop 'spacing 64)
    (prop 'x (bound '(this width) (lambda (w) (/ (- 800 w) 2))))
    (prop 'y (bound '(this height) (lambda (h) (/ (- 480 h) 2))))
    (kitt-7)
    (kitt-9)))