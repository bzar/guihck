(define menu-button
  (composite text
    (prop 'size 32)
    (alias 'on-click 'ma 'on-click)
    (prop 'color (bound '(ma hover ma pressed) (lambda (hover pressed) 
      (cond 
        (pressed '(192 192 255)) 
        (hover '(128 128 255)) 
        (else '(64 64 255))))))
    
    (mouse-area
      (id 'ma)
      (fill-parent))))

(define splash-screen
  (composite rectangle
    (prop 'color '(16 16 16))
    
    (column
      (align 'center)
      (prop 'spacing 16)
    
      (text
        (prop 'text "PONG!")
        (prop 'size 42))
      
      (menu-button
        (prop 'text "New Game")
        (prop 'size 32)
        (method 'on-click (lambda (b x y) 
          (set-prop! (find-element 'app) 'view "game"))))
        
      (menu-button
        (prop 'text "Quit")
        (prop 'size 32)
        (method 'on-click (lambda (b x y) 
          (quit)))))))

(define (game-gen)
  (define (input key scancode action mods)
    (cond 
      ((= key (keyboard 'a))
        (if (eq? action 'release)
          (set-prop! (find-element 'bat-1) 'vy 0)
          (set-prop! (find-element 'bat-1) 'vy (- (get-prop 'speed)))))
      ((= key (keyboard 'z))
        (if (eq? action 'release)
          (set-prop! (find-element 'bat-1) 'vy 0)
          (set-prop! (find-element 'bat-1) 'vy (get-prop 'speed))))
      ((= key (keyboard 'up))
        (if (eq? action 'release)
          (set-prop! (find-element 'bat-2) 'vy 0)
          (set-prop! (find-element 'bat-2) 'vy (- (get-prop 'speed)))))
      ((= key (keyboard 'down))
        (if (eq? action 'release)
          (set-prop! (find-element 'bat-2) 'vy 0)
          (set-prop! (find-element 'bat-2) 'vy (get-prop 'speed))))
      ((= key (keyboard 'escape)) 
        (quit))))
        
  (define (update)
    (define (limit-value element property lower-bound upper-bound)
      (let ((value (get-prop element property)))
        (cond 
          ((< value lower-bound)
            (set-prop! element property lower-bound)
            #t)
          ((> value upper-bound)
            (set-prop! element property upper-bound)
            #t)
          (else #f))))
          
    (define (bat-hit bat ball)
      (if (and (>= (+ (get-prop ball 'x) (get-prop ball 'width)) (get-prop bat 'x))
               (>= (+ (get-prop bat 'x) (get-prop bat 'width)) (get-prop ball 'x))
               (>= (+ (get-prop ball 'y) (get-prop ball 'height)) (get-prop bat 'y))
               (>= (+ (get-prop bat 'y) (get-prop bat 'height)) (get-prop ball 'y)))
        (begin
          (if (< (get-prop ball 'vx) 1)
            (set-prop! ball 'x (+ (get-prop bat 'x) (get-prop bat 'width)))
            (set-prop! ball 'x (- (get-prop bat 'x) (get-prop ball 'width))))
          (set-prop! ball 'vx (- (get-prop ball 'vx)))
          #t)))
    
    (define (goal ball player)
      (set-prop! ball 'x (/ (get-prop 'width) 2))
      (set-prop! ball 'y (/ (get-prop 'height) 2))
      (let ((score-text (find-element (if (= player 1) 'score-1 'score-2))))
        (set-prop! score-text 'score (+ (get-prop score-text 'score) 1))))
            
    (define (check-goal ball lower-bound upper-bound)
      (let ((x (get-prop ball 'x)))
        (cond 
          ((< x lower-bound) (goal ball 2))
          ((> x upper-bound) (goal ball 1))
          (else #f))))
    
    (let ((bat-1 (find-element 'bat-1))
          (bat-2 (find-element 'bat-2))
          (ball (find-element 'ball))
          (width (get-prop 'width))
          (height (get-prop 'height)))
    
      (set-prop! bat-1 'y (+ (get-prop bat-1 'y) (get-prop bat-1 'vy)))
      (set-prop! bat-2 'y (+ (get-prop bat-2 'y) (get-prop bat-2 'vy)))
      (set-prop! ball 'x (+ (get-prop ball 'x) (get-prop ball 'vx)))
      (set-prop! ball 'y (+ (get-prop ball 'y) (get-prop ball 'vy)))
    
      (limit-value bat-1 'y 0 (- height (get-prop bat-1 'height)))
      (limit-value bat-2 'y 0 (- height (get-prop bat-2 'height)))
      
      (if (limit-value ball 'y 0 (- height (get-prop ball 'height)))
        (set-prop! ball 'vy (- (get-prop ball 'vy))))
        
      (bat-hit bat-1 ball)
      (bat-hit bat-2 ball)
      
      (check-goal ball (- (get-prop bat-1 'width)) width)))
    
  (composite rectangle
    (id 'game)
    (prop 'color '(16 16 16))
    (method 'update update)
    (method 'on-key input)
    (prop 'speed 3)
    
    (text
      (id 'score-1)
      (align 'top-left 10)
      (prop 'score 0)
      (prop 'size 40)
      (prop 'text (bound '(this score) object->string)))
      
    (text
      (id 'score-2)
      (align 'top-right 10)
      (prop 'score 0)
      (prop 'size 40)
      (prop 'text (bound '(this score) object->string)))
      
    (rectangle
      (id 'ball)
      (prop 'width 20)
      (prop 'height 20)
      (align 'center)
      (prop 'vx 3)
      (prop 'vy 3))
      
    (rectangle
      (id 'bat-1)
      (prop 'width 20)
      (prop 'height 100)
      (align 'left 20)
      (align 'vertical-center)
      (prop 'vy 0))
      
    (rectangle
      (id 'bat-2)
      (prop 'width 20)
      (align 'height 100)
      (align 'right 20)
      (align 'vertical-center)
      (prop 'vy 0))
      
    (timer
      (prop 'running (bound '(game visible)))
      (prop 'interval (/ 1 60))
      (prop 'repeat -1)
      (method 'on-timeout (lambda (c) (call (parent) 'update))))
    ))
(define game (game-gen))

(define (view name constructor . args)
  (apply constructor
    (cons (prop 'visible (bound '(parent view) (lambda (view) (equal? view name)))) 
          args)))
    
(create-elements!
  (item
    (id 'app)
    (fill-parent)
    (prop 'view "splash-screen")
    (view "splash-screen" splash-screen
      (fill-parent))
    (view "game" game
      (fill-parent))))

(focus! (find-element 'game))