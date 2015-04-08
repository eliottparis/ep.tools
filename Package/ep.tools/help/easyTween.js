/*
Adaptation du code Jquery 1.3 pour MaxMsp par Eliott PARIS.
05/2011
*/


////////////////////////////////general settings
inlets =1;
outlets =2;

//////////// Declaration de variables.

// t: temps actuel, b: valeur de depart, c: valeur d'arrivee, d: duree totale de l'interpolation.

var b = 0.
var c = 1.
var d = 1.
var s = 1.70158 // sert a certaines fonctions (elastiv, Bounce..)

var mod = "Sine"
var iz = "easeInOut"
var easeMode = iz+mod

//////////// initialisation des variables :

function loadbang (){
init ();
outlet (1, easeMode);
};

function init () {
b = 0.;
c = 1.;
d = 1.;
mod = "Sine";
iz = "easeInOut";
easeMode = iz+mod;
}

function changeParam (depart, arrive, duree) {
b = depart;
c = arrive;
d = duree;
}

function mode(symb){
mod = symb;
easeMode = iz+mod;
};

function ease(symb){
iz = symb;
easeMode = iz+mod;
};

///////////////////// Dump sort les parametres actuels de l'objet dans l'outlet 2 et les post dans Max window.
function dump () {
post ("Tween params: \nvaleur de depart = "+b+"\nvaleur d'arrivee = "+c+"\nduree = "+d+"\nmode : "+iz+mod+"\n");
outlet (1, easeMode, b, c, d);
}


function msg_float(t){
    var callFunc = easeMode;
    switch(callFunc)
    {
    
  ///  
        case "easeInLinear":
                Linear(t);
                break;
        case "easeOutLinear":
                Linear(t);
                break;
        case "easeInOutLinear":
                Linear(t);
                break;
  ///  
        case "easeInSine":
                easeInSine(t);
                break;
        case "easeOutSine":
                easeOutSine(t);
                break;
        case "easeInOutSine":
                easeInOutSine(t);
                break;
  ///              
        case "easeInQuart":
                easeInQuart(t);
                break;
        case "easeOutQuart":
                easeOutQuart(t);
                break;
        case "easeInOutQuart":
                easeInOutQuart(t);
                break;
  ///              
        case "easeInQuint":
                easeInQuint(t);
                break;
        case "easeOutQuint":
                easeOutQuint(t);
                break;
        case "easeInOutQuint":
                easeInOutQuint(t);
                break;
  ///              
        case "easeInExpo":
                easeInExpo(t);
                break;
        case "easeOutExpo":
                easeOutExpo(t);
                break;
        case "easeInOutExpo":
                easeInOutExpo(t);
                break;
  ///              
        case "easeInCubic":
                easeInCubic(t);
                break;
        case "easeOutCubic":
                easeOutCubic(t);
                break;
        case "easeInOutCubic":
                easeInOutCubic(t);
                break;
  ///
        case "easeInBack":
                easeInBack(t);
                break;
        case "easeOutBack":
                easeOutBack(t);
                break;
        case "easeInOutBack":
                easeInOutBack(t);
                break;
  ///  
        case "easeInCirc":
                easeInCirc(t);
                break;
        case "easeOutCirc":
                easeOutCirc(t);
                break;
        case "easeInOutCirc":
                easeInOutCirc(t);
                break;
  ///  
        case "easeInQuad":
                easeInQuad(t);
                break;
        case "easeOutQuad":
                easeOutQuad(t);
                break;
        case "easeInOutQuad":
                easeInOutQuad(t);
                break;
  ///  
        case "easeInElastic":
                easeInElastic(t);
                break;
        case "easeOutElastic":
                easeOutElastic(t);
                break;
        case "easeInOutElastic":
                easeInOutElastic(t);
                break;
  ///  
        case "easeInBounce":
                easeInBounce(t);
                break;
        case "easeOutBounce":
                easeOutBounce(t);
                break;
        case "easeInOutBounce":
                easeInOutBounce(t);
                break;
  ///  
        default:
                easeInSine(t);
                break;
    }
}


////////////////////////////////////////// Linear :
    function Linear (t) {
    outlet (0,t);
    }

//////////////////////////////////////////

// fonctions Jquery a la sauce Max implementees

////////////////////////////////////////// Quart :
    function easeInQuart (t) {
    output= c*(t/=d)*t*t*t + b;
    outlet (0,output);
    }

    function easeOutQuart (t) {
    output= -c * ((t=t/d-1)*t*t*t - 1) + b;
    outlet (0,output);
    }
    
    function easeInOutQuart (t) {
    if ((t/=d/2) < 1) output= c/2*t*t*t*t + b;
    else
    output= -c/2 * ((t-=2)*t*t*t - 2) + b;
    outlet (0,output);
    }

////////////////////////////////////////// Sine :
    function easeInSine (t) {
    output= -c * Math.cos(t/d * (Math.PI/2)) + c + b;
    outlet (0,output);
    }

    function easeOutSine (t) {
    output= c * Math.sin(t/d * (Math.PI/2)) + b;
    outlet (0,output);
    }
    
    function easeInOutSine (t) {
    output= -c/2 * (Math.cos(Math.PI*t/d) - 1) + b;
    outlet (0,output);
    }

////////////////////////////////////////// Quint :
    function easeInQuint (t) {
    output= c*(t/=d)*t*t*t*t + b;
    outlet (0,output);
    }
    
    function easeOutQuint (t) {
    output= c*((t=t/d-1)*t*t*t*t + 1) + b;
    outlet (0,output);
    }
    
    function easeInOutQuint (t) {
    if ((t/=d/2) < 1) output= c/2*t*t*t*t*t + b;
    else
    output= c/2*((t-=2)*t*t*t*t + 2) + b;
    outlet (0,output);
    }

////////////////////////////////////////// Expo :
    function easeInExpo (t) {
    output= (t==0) ? b : c * Math.pow(2, 10 * (t/d - 1)) + b;
    outlet (0,output);
    }
    
    function easeOutExpo (t) {
    output= (t==d) ? b+c : c * (-Math.pow(2, -10 * t/d) + 1) + b;
    outlet (0,output);
    }
    
    function easeInOutExpo (t) {
        if (t==0) output= b;
        else if (t==d) output= b+c;
        else if ((t/=d/2) < 1) output= c/2 * Math.pow(2, 10 * (t - 1)) + b;
        else
    output= c/2 * (-Math.pow(2, -10 * --t) + 2) + b;
    outlet (0,output);
    }

////////////////////////////////////////// Cubic :
    function easeInCubic (t) {
    output= c*(t/=d)*t*t + b;
    outlet (0,output);
    }
    
    function easeOutCubic (t) {
    output= c*((t=t/d-1)*t*t + 1) + b;
    outlet (0,output);
    }
    
    function easeInOutCubic (t) {
        if ((t/=d/2) < 1) output= c/2*t*t*t + b;
        else
    output= c/2*((t-=2)*t*t + 2) + b;
    outlet (0,output);
    }

////////////////////////////////////////// Back :
    function easeInBack (t) {
    s = 1.70158;
    output= c*(t/=d)*t*((s+1)*t - s) + b;
    outlet (0,output);
    }
    
    function easeOutBack (t) {
    s = 1.70158; 
    output= c*((t=t/d-1)*t*((s+1)*t + s) + 1) + b;
    outlet (0,output);
    }
    
    function easeInOutBack (t) {
    s = 1.70158;
    if ((t/=d/2) < 1) output= c/2*(t*t*(((s*=(1.525))+1)*t - s)) + b;
    else
    output= c/2*((t-=2)*t*(((s*=(1.525))+1)*t + s) + 2) + b;
    outlet (0,output);
    }


////////////////////////////////////////// Circ :
    function easeInCirc (t) {
    output= -c * (Math.sqrt(1 - (t/=d)*t) - 1) + b;
    outlet (0,output);
    };
    
    function easeOutCirc (t) {
    output= c * Math.sqrt(1 - (t=t/d-1)*t) + b;
    outlet (0,output);
    };
    
    function easeInOutCirc (t) {
    if ((t/=d/2) < 1) output= -c/2 * (Math.sqrt(1 - t*t) - 1) + b;
    else
    output= c/2 * (Math.sqrt(1 - (t-=2)*t) + 1) + b;
    outlet (0,output);
    };


////////////////////////////////////////// Quad :
    function easeInQuad (t) {
    output= c*(t/=d)*t + b;
    outlet (0,output);
    };
    
    function easeOutQuad (t) {
    output= -c *(t/=d)*(t-2) + b;
    outlet (0,output);
    };
    
    function easeInOutQuad (t) {
    if ((t/=d/2) < 1) output= c/2*t*t + b;
    else
    output= -c/2 * ((--t)*(t-2) - 1) + b;
    outlet (0,output);
    };



////////////////////////////////////////// Elastic :
    function easeInElastic (t) {
        var s=1.70158; var p=0; var a=c;
        if (t==0) output= b; if ((t/=d)==1) output= b+c; else  if (!p) p=d*.3;
        if (a < Math.abs(c)) { a=c; var s=p/4; }
        else var s = p/(2*Math.PI) * Math.asin (c/a);
        output= -(a*Math.pow(2,10*(t-=1)) * Math.sin( (t*d-s)*(2*Math.PI)/p )) + b;
    outlet (0,output);
    };
    
    function easeOutElastic (t) {
        var s=1.70158; var p=0; var a=c;
        if (t==0) output= b; if ((t/=d)==1) output= b+c; else if (!p) p=d*.3;
        if (a < Math.abs(c)) { a=c; var s=p/4; }
        else var s = p/(2*Math.PI) * Math.asin (c/a);
        output= a*Math.pow(2,-10*t) * Math.sin( (t*d-s)*(2*Math.PI)/p ) + c + b;
    outlet (0,output);
    };
    
    function easeInOutElastic (t) {
        var s=1.70158; var p=0; var a=c;
        if (t==0) output= b; if ((t/=d/2)==2) output= b+c; else if (!p) p=d*(.3*1.5);
        if (a < Math.abs(c)) { a=c; var s=p/4; }
        else var s = p/(2*Math.PI) * Math.asin (c/a);
        if (t < 1) output= -.5*(a*Math.pow(2,10*(t-=1)) * Math.sin( (t*d-s)*(2*Math.PI)/p )) + b;
        else
        output= a*Math.pow(2,-10*(t-=1)) * Math.sin( (t*d-s)*(2*Math.PI)/p )*.5 + c + b;
    outlet (0,output);
    };




////////////////////////////////////////// Bounce :
    function bounce (t, b, c, d){
            if ((t/=d) < (1 / 2.75))
                return  c*(7.5625 * t * t) + b;

            else if (t < (2 / 2.75))
                return c*( 7.5625 * (t -= (1.5 / 2.75)) * t + 0.75) + b ;

            else if (t < (2.5 / 2.75))
                return ( 7.5625 * (t -= (2.25 / 2.75)) * t + 0.9375) + b;
            
            else
                return c*( 7.5625 * (t -= (2.625 / 2.75)) * t + 0.984375) + b;
    }

    ////////////////////////////////
    function easeInBounce (t){
    output= 1- bounce(d-t, 0, c, d) + b;
    outlet(0,output);
    }

    ////////////////////////////////
    function easeOutBounce (t){
    output= bounce(t, b, c, d);
    outlet(0,output);
    }

    ////////////////////////////////
    function easeInOutBounce(t){
        if (t < d/2)
            output= bounce (t*2, 0, c, d) * .5 + b;
        else
            output= bounce (t*2-d, 0, c, d) * .5 + c*.5 + b;
    outlet(0,output);
    }

