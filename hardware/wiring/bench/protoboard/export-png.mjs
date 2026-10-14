// Offline raster export. Pass a folder containing node_modules/puppeteer.
import {createRequire} from 'node:module';
import {readFile,writeFile} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import {fileURLToPath} from 'node:url';
import path from 'node:path';
const here=path.join(path.dirname(fileURLToPath(import.meta.url)),'generated');
const manifest={renderer:'Puppeteer', node:process.version, sources:{}, files:{}};
if(!process.argv[2]) throw new Error('Usage: node export-png.mjs <npm-tools-root>');
const require=createRequire(path.join(path.resolve(process.argv[2]),'package.json'));
const puppeteer=require('puppeteer');
manifest.puppeteer=require('puppeteer/package.json').version;
for(const name of ['generate-plan.py','render-board.py','export-png.mjs','../bench-harness.yml']) {
  const bytes=await readFile(path.resolve(here,'..',name));
  manifest.sources[name]=createHash('sha256').update(bytes).digest('hex');
}
const browser=await puppeteer.launch({headless:true});
manifest.browser=await browser.version();
try {
  const page=await browser.newPage();
  await page.setViewport({width:1900,height:1410,deviceScaleFactor:1.5});
  await page.setRequestInterception(true);
  page.on('request',req=>req.url().startsWith('data:')?req.continue():req.abort());
  for(const name of ['board-overview','board-ground-power','board-digital','board-sensor-button']) {
    const svg=await readFile(path.join(here,name+'.svg'),'utf8');
    await page.setContent('<html><body style="margin:0">'+svg+'</body></html>');
    await page.evaluate(()=>document.fonts.ready);
    await page.screenshot({path:path.join(here,name+'.png')});
    for(const ext of ['svg','png']) {
      const bytes=await readFile(path.join(here,name+'.'+ext));
      manifest.files[name+'.'+ext]=createHash('sha256').update(bytes).digest('hex');
    }
    console.log(name+'.png');
  }
  await writeFile(path.join(here,'png-generation.json'),JSON.stringify(manifest,null,2)+'\n');
} finally {await browser.close();}
