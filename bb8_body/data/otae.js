class Otae {
    constructor(autoPopulate) {
        this.autoPopulate = autoPopulate;
    }

    registerEvents(categories) {
        var source = new EventSource('/events');

        source.addEventListener('open', function(e) {
            console.log("Events Connected");
        }, false);

        source.addEventListener('error', function(e) {
            if (e.target.readyState != EventSource.OPEN) {
                console.log("Events Disconnected");
            }
        }, false);
        
        categories.forEach(ev => {
            source.addEventListener(ev, (e) => this.eventReceived(ev, e.data), false);
        });
    }

    eventReceived(cat,str) {
        var jobj = JSON.parse(str);
        this.variableReceived(cat, "", jobj);
    }

    variableReceived(cat, name, x) {
        if(Array.isArray(x)) {
            for(var i=0; i<x.length; i++) {
                this.variableReceived(cat, name+"_"+i, x[i]);
            }
        } else if(typeof x === "object") {
            for (var prop in x) {
                var n = (name != "") ? name + "_" + prop : prop;
                this.variableReceived(cat, n, x[prop]);
            }
        } else {
            this.updateVariable(cat, name, x);
        }
    }

    updateVariable(cat,name,value) {
        var key = cat+"_"+name;
        var elem = document.getElementById(key);
        if(elem == null && this.autoPopulate) {
            var disp = document.getElementById(cat);
            elem = this.createInput(disp,cat,name);
        }
        if(elem != null) {
            elem.innerHTML = value;
        }
    }

    createInput(disp,cat,name) {
        var elem = null;
        if(disp != null) {
            var key = cat+"_"+name;
            var header = document.createElement("label");
            header.innerHTML = name + ": ";
            disp.appendChild(header);
            elem = document.createElement("label");
            elem.setAttribute("id", key);
            disp.appendChild(elem);
            var input = document.createElement("input");
            input.setAttribute("id", "input_"+key);
            input.addEventListener("keypress", (event) =>{
                if (event.key === "Enter") {
                  event.preventDefault();
                  this.setVariable(cat,name)
                }
            });
            disp.appendChild(input);
            var btn = document.createElement("button");
            btn.addEventListener("click", ()=>this.setVariable(cat,name));
            btn.innerHTML = "Set";
            disp.appendChild(btn);
            var br = document.createElement("br");
            disp.appendChild(br);
        }
        return elem;
    }

    setVariable(cat,name) {
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
}
