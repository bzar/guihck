(define (composite constructor props . children)
  (lambda (p . c) 
    (apply constructor (append (list (append props p)) children c))))
  
(define color-button
  (composite rectangle '()
    (mouse-area (append props '(onMouseDown (set-clear-color (get-element-property! 'color)))))))
      
(define row
  (composite item '(xChanged (display "x changed")
                    yChanged (display "y changed"))))

(create-elements
  (row '(x 100 y 100)
    (color-button '(color (255 0 0) width 50 height 50))
    (color-button '(color (0 255 0) width 60 height 60))
    (color-button '(color (0 0 255) width 70 height 70))))
                  
    
