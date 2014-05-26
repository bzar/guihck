(define kitt-part
  (composite rectangle
    (prop 'width 64)
    (prop 'height 64)
    (prop 'phase 1)
    (prop 'color (bound '(this phase) (lambda (phase)
      (list (* phase 32) 0 0))))))
      
(define (kitt-gen)
  (define (kitt-phase i cur n)
    (- n (abs (- i cur))))
    
  (define (kitt-bound i)
    (bound '(parent phase) (lambda (cur)
      (kitt-phase i cur 7))))
      
  (composite row
    (prop 'phase 1)
    (prop 'direction 1)
    (kitt-part (prop 'phase (kitt-bound 1)))
    (kitt-part (prop 'phase (kitt-bound 2)))
    (kitt-part (prop 'phase (kitt-bound 3)))
    (kitt-part (prop 'phase (kitt-bound 4)))
    (kitt-part (prop 'phase (kitt-bound 5)))
    (kitt-part (prop 'phase (kitt-bound 6)))
    (kitt-part (prop 'phase (kitt-bound 7)))
    (timer
      (prop 'interval 0.1)
      (prop 'running #t)
      (prop 'repeat -1)
      (prop 'on-timeout (lambda (c)
        (let ((next-phase (+ (get-prop (parent) 'phase)
                             (get-prop (parent) 'direction))))
          (cond ((> next-phase 7)
                 (begin
                   (set-prop! (parent) 'phase 6)
                   (set-prop! (parent) 'direction -1)))
                ((< next-phase 1)
                 (begin
                   (set-prop! (parent) 'phase 2)
                   (set-prop! (parent) 'direction 1)))
                (else (set-prop! (parent) 'phase next-phase)))))))))
(define kitt (kitt-gen))

(create-elements!
  (kitt
    (prop 'x (bound '(this width) (lambda (w) (/ (- 800 w) 2))))
    (prop 'y (bound '(this height) (lambda (h) (/ (- 480 h) 2))))))