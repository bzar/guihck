(define (color-button props)
  (rectangle props
    (mouse-area (append props '(onMouseDown (set-clear-color (get-element-property! 'color)))))))
      
(create-elements
  (color-button '(color (255 0 0) x 100 y 100 width 50 height 50))
  (color-button '(color (0 255 0) x 200 y 100 width 50 height 50))
  (color-button '(color (0 0 255) x 300 y 100 width 50 height 50)))
                  
    
