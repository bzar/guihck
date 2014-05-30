
(define menu-button
  (composite text
    (prop 'text "")
    (prop 'size 32)
    (prop 'highlight #f)
    (prop 'on-click '(alias (find-element 'ma) 'on-mouse-up))
    (prop 'color (bound '(this highlight) (lambda (highlighted) (if highlighted '(128 128 255) '(64 64 255)))))
    
    (mouse-area
      (id 'ma)
      (prop 'width '(alias (parent) 'width))
      (prop 'height '(alias (parent) 'height))
      (prop 'on-mouse-enter (lambda (sx sy dx dy) (set-prop! (parent) 'highlight #t)))
      (prop 'on-mouse-exit (lambda (sx sy dx dy) (set-prop! (parent) 'highlight #f))))))

(define splash-screen
  (composite rectangle
    (prop 'width 800)
    (prop 'height 480)
    (prop 'color '(16 16 16))
    
    (column
      (prop 'x (bound '(this width parent width) (lambda (w pw) (/ (- pw w) 2))))
      (prop 'y (bound '(this height parent height) (lambda (h ph) (/ (- ph h) 2))))
      (prop 'spacing 16)
    
      (text
        (prop 'text "PONG!")
        (prop 'size 42))
      
      (menu-button
        (prop 'text "New Game")
        (prop 'size 32)
        (prop 'on-click (lambda (b x y) 
          (set-prop! (find-element 'app) 'view "game"))))
        
      (menu-button
        (prop 'text "Quit")
        (prop 'size 32)
        (prop 'on-click (lambda (b x y) 
          (quit)))))))

(define game
  (composite rectangle
    (prop 'width 800)
    (prop 'height 480)
    (prop 'color '(16 16 16))
    
    (rectangle
      (id 'ball)
      (prop 'width 20)
      (prop 'height 20)
      (prop 'x (- (/ 800 2) (/ 20 2)))
      (prop 'y (- (/ 480 2) (/ 20 2))))
    
    
    (rectangle
      (id 'bat-1)
      (prop 'width 20)
      (prop 'height 100)
      (prop 'x 20)
      (prop 'y (- (/ 480 2) (/ 100 2))))
    (rectangle
      (id 'bat-2)
      (prop 'width 20)
      (prop 'height 100)
      (prop 'x (- 800 (* 2 20)))
      (prop 'y (- (/ 480 2) (/ 100 2))))
    ))

(define (view name constructor)
  (constructor
    (prop 'visible (bound '(parent view) (lambda (view) (equal? view name))))))
    
(create-elements!
  (item
    (id 'app)
    (prop 'view "splash-screen")
    (view "splash-screen" splash-screen)
    (view "game" game)))
      