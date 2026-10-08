import {createRequire} from 'node:module';
import {pathToFileURL} from 'node:url';
import {resolve} from 'node:path';
import {readFileSync,readdirSync} from 'node:fs';
// Optional preview tooling lives outside the tracked project.
const toolsRoot=resolve(process.argv[2] || '_staging/work/wokwi/tools');
const req=createRequire(pathToFileURL(resolve(toolsRoot,'package.json')));
const {default:puppeteer}=await import(pathToFileURL(req.resolve('puppeteer')));
const browser=await puppeteer.launch({headless:true});
try{
 const page=await browser.newPage(); await page.setViewport({width:2300,height:1500});
 await page.goto('https://wokwi.com/projects/new/esp32-s3',{waitUntil:'domcontentloaded',timeout:60000});
 await page.waitForSelector('.monaco-editor');
 await page.locator('[aria-label="toggle dark mode"]').click();
 const chipNames=readdirSync('simulation/wokwi/chips').filter(n=>n.endsWith('.chip.json')).map(n=>n.replace('.chip.json',''));
 const setFile=async(name,text)=>{
   await page.evaluate(name=>Array.from(document.querySelectorAll('*')).find(el=>el.textContent===name&&el.children.length===0)?.click(),name);
   await page.waitForFunction(name=>window.monaco?.editor.getModels().some(m=>m.uri.toString()===`vfs:${name}`),{},name);
   await page.evaluate(({name,text})=>window.monaco.editor.getModels().find(m=>m.uri.toString()===`vfs:${name}`).setValue(text),{name,text});
 };
 for(const name of chipNames){
   await page.locator('[aria-label="Add a new part"]').click();
   await page.locator('::-p-text(Custom Chip)').click();
   await page.waitForSelector('[role="dialog"] input[type="text"]');
   await page.type('[role="dialog"] input[type="text"]',name);
   const button=await page.evaluateHandle(()=>Array.from(document.querySelectorAll('[role="dialog"] button')).find(el=>el.textContent.toLowerCase().includes('create chip')));
   await button.click();
   await page.waitForFunction(()=>!document.querySelector('[role="dialog"]'));
   for(const suffix of ['.chip.c','.chip.json'])await setFile(name+suffix,readFileSync(`simulation/wokwi/chips/${name}${suffix}`,'utf8'));
   console.log('Loaded',name);
   await setFile('diagram.json',JSON.stringify({version:1,author:'LS PCV System contributors',editor:'wokwi',parts:[{type:'board-esp32-s3-devkitc-1',id:'J1',top:40,left:520,attrs:{}}],connections:[]}));
   await new Promise(r=>setTimeout(r,250));
 }
 await setFile('diagram.json',readFileSync('simulation/wokwi/diagram.json','utf8'));
 await new Promise(r=>setTimeout(r,2000));
 await page.evaluate(()=>Array.from(document.querySelectorAll('*')).find(el=>el.textContent==='sketch.ino'&&el.children.length===0)?.click());
 await page.evaluate(()=>{document.getElementById('S1').parentElement.parentElement.style.transform='matrix(1, 0, 0, 1, 50, 180)';});
 await new Promise(r=>setTimeout(r,500));
 const result=await page.evaluate(()=>({title:document.title,diagnostics:window.monaco.editor.getModelMarkers({}).map(m=>({uri:m.resource.toString(),message:m.message,severity:m.severity})),missing:document.body.innerText.match(/Missing chip[^\n]*/g)||[],parts:Array.from(document.querySelectorAll('[id]')).filter(el=>['J1','J2','J3','LS1','RAILS','FTP','R1','C1','S1'].includes(el.id)).map(el=>({id:el.id,type:el.tagName,bounds:el.getBoundingClientRect().toJSON(),pins:el.pinInfo})),text:document.body.innerText.slice(-1800)}));
 if(result.missing.length || result.diagnostics.length || result.parts.length !== 9) throw new Error('Incomplete or invalid diagram render');
 console.log('PASS: all nine electrical parts rendered; no missing chips or editor diagnostics.');


 // Capture only the illustration, omitting editor UI and unrelated template code.
 await page.screenshot({path:'simulation/wokwi/diagram-preview.png',clip:{x:1180,y:140,width:1120,height:1360}});
}catch(error){
 console.log('Failure UI:',await (await browser.pages()).at(-1).evaluate(()=>document.body.innerText.slice(-2500)));

 throw error;
}finally{await browser.close();}
