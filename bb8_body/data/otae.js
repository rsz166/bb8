if (!!window.EventSource) {
    var source = new EventSource('/events');

    source.addEventListener('open', function(e) {
        console.log("Events Connected");
    }, false);

    source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
        console.log("Events Disconnected");
    }
    }, false);
    
    ["regs","config","tuning"].forEach(ev => {
        source.addEventListener(ev, (e) => eventReceived(ev, e.data), false);
    });
}

function eventReceived(cat,str) {
    var jobj = JSON.parse(str);
    variableReceived(cat, "", jobj);
}
function variableReceived(cat, name, x) {
    if(Array.isArray(x)) {
        for(var i=0; i<x.length; i++) {
            variableReceived(cat, name+"_"+i, x[i]);
        }
    } else if(typeof x === "object") {
        for (var prop in x) {
            var n = (name != "") ? name + "_" + prop : prop;
            variableReceived(cat, n, x[prop]);
        }
    } else {
        updateVariable(cat, name, x);
    }
}
function updateVariable(cat,name,value) {
    var key = cat+"_"+name;
    var elem = document.getElementById(key);
    if(elem == null) {
        var disp = document.getElementById(cat);
        if(disp != null) {
            var header = document.createElement("label");
            header.innerHTML = name + ": ";
            disp.appendChild(header);
            elem = document.createElement("label");
            elem.setAttribute("id", key);
            disp.appendChild(elem);
            var input = document.createElement("input");
            input.setAttribute("id", "input_"+key);
            disp.appendChild(input);
            var btn = document.createElement("button");
            btn.addEventListener("click", ()=>setVariable(cat,name));
            btn.innerHTML = "Set";
            disp.appendChild(btn);
            var br = document.createElement("br");
            disp.appendChild(br);
        }
    }
    elem.innerHTML = value;
}
function setVariable(cat,name) {
    var input = document.getElementById("input_"+cat+"_"+name);
    if(input != null) {
        var value = input.value;
        fetch("/set?"+cat+"_"+name+"="+value)
        .then( response => {
            if(response.ok) {
                input.value = "";
                input.style.backgroundColor = "lightgreen";
            } else {
                input.style.backgroundColor = "lightcoral";
            }
        });
    }
}