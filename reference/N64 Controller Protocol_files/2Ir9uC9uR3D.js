/*1369716875,178129763*/

if (self.CavalryLogger) { CavalryLogger.start_js(["pb4A2"]); }

__d("NotificationURI",["URI"],function(a,b,c,d,e,f){var g=b('URI'),h={localize:function(i){i=g(i);if(!i.isFacebookURI())return i.toString();var j=i.getSubdomain();return i.getUnqualifiedURI().getQualifiedURI().setSubdomain(j).toString();},snowliftable:function(i){if(!i)return false;i=g(i);return i.isFacebookURI()&&i.getQueryData().hasOwnProperty('fbid');}};e.exports=h;});
__d("legacy:onload-action",["OnloadHooks"],function(a,b,c,d){var e=b('OnloadHooks');a._onloadHook=e._onloadHook;a._onafterloadHook=e._onafterloadHook;a.runHook=e.runHook;a.runHooks=e.runHooks;a.keep_window_set_as_loaded=e.keepWindowSetAsLoaded;},3);
__d("MercuryFilters",["MessagingTag","arrayContains"],function(a,b,c,d,e,f){var g=b('MessagingTag'),h=b('arrayContains'),i=[g.UNREAD],j={ALL:'all',getSupportedFilters:function(){return i.concat();},isSupportedFilter:function(k){return h(i,k);}};e.exports=j;});
__d("MercuryOrderedThreadlist",["JSLogger","MercuryActionTypeConstants","MercuryFilters","MercuryFolders","MercuryPayloadSource","MercurySingletonMixin","MessagingTag","RangedCallbackManager","MercuryServerRequests","MercuryThreadInformer","MercuryThreads","arrayContains","copyProperties"],function(a,b,c,d,e,f){var g=b('JSLogger'),h=b('MercuryActionTypeConstants'),i=b('MercuryFilters'),j=b('MercuryFolders'),k=b('MercuryPayloadSource'),l=b('MercurySingletonMixin'),m=b('MessagingTag'),n=b('RangedCallbackManager'),o=b('MercuryServerRequests'),p=b('MercuryThreadInformer'),q=b('MercuryThreads'),r=b('arrayContains'),s=b('copyProperties'),t=g.create('ordered_threadlist_model'),u=i.getSupportedFilters().concat([i.ALL]),v=j.getSupportedFolders();function w(ga,ha,ia,ja){var ka=[],la=false,ma=false,na=(ha||[]).filter(function(ua){var va=ua.filter||i.ALL;return (ua.folder==ia||(!ua.folder&&ia==m.INBOX))&&va==ja;}),oa=ga._threadlistOrderMap[ia][ja].getAllResources(),pa;na.forEach(function(ua){ka=ka.concat(ua.thread_ids);if(ua.error){if(ua.end>oa.length)ma=ua.error;}else{var va=ua.end-ua.start;if(ua.thread_ids.length<va)la=true;}if(!pa||ua.end>pa)pa=ua.end;});aa(ga,ka,ia,ja);if(la){ga._threadlistOrderMap[ia][ja].setReachedEndOfArray();}else if(ma){ga._threadlistOrderMap[ia][ja].setError(ma);}else{var qa=ga._threadlistOrderMap[ia][ja].getCurrentArraySize();if(pa&&qa<pa){var ra={},sa=oa.concat(ka),ta=sa.filter(function(ua){var va=ra[ua];ra[ua]=true;return va;});if(ta.length){t.warn('duplicate_threads',{folder:ia,expected_final_size:pa,actual_final_size:qa,duplicate_thread_ids:ta});ga._serverRequests.fetchThreadlistInfo(pa,ta.length,ia,ja==i.ALL?null:ja);}}}}function x(ga,ha){v.forEach(function(ia){u.forEach(function(ja){w(ga,ha,ia,ja);});});}function y(ga,ha){var ia={};v.forEach(function(ja){ia[ja]={};u.forEach(function(ka){ia[ja][ka]={to_add:[],to_remove:[],to_remove_if_last_after_add:{}};});});ha.forEach(function(ja){if(ja.is_forward)return;var ka=ja.thread_id,la=ba(ga,ka),ma=ca(ga,ka);function na(){ma.forEach(function(pa){ia[la][pa].to_add.push(ka);if(!ga._threadlistOrderMap[la][pa].hasReachedEndOfArray()&&!ga._threadlistOrderMap[la][pa].hasResource(ka))ia[la][pa].to_remove_if_last_after_add[ka]=true;});}function oa(pa){if(r(u,pa))if(r(ma,pa)){ia[la][pa].to_add.push(ka);}else ia[la][pa].to_remove.push(ka);}if(!la){if(ja.action_type===h.CHANGE_FOLDER||ja.action_type===h.CHANGE_ARCHIVED_STATUS)v.forEach(function(pa){if(pa!==la)u.forEach(function(qa){ga._threadlistOrderMap[pa][qa].removeResources([ka]);});});return;}switch(ja.action_type){case h.DELETE_THREAD:ma.forEach(function(pa){ia[la][pa].to_remove.push(ka);});break;case h.CHANGE_ARCHIVED_STATUS:case h.CHANGE_FOLDER:na();break;case h.CHANGE_READ_STATUS:oa(m.UNREAD);break;case h.USER_GENERATED_MESSAGE:case h.LOG_MESSAGE:ma.forEach(function(pa){if(da(ga,ka,la,pa))ia[la][pa].to_add.push(ka);});break;}});v.forEach(function(ja){u.forEach(function(ka){var la=ga._threadlistOrderMap[ja][ka];aa(ga,ia[ja][ka].to_add,ja,ka);for(var ma=la.getCurrentArraySize()-1;ma>=0;ma--){var na=la.getResourceAtIndex(ma);if(!ia[ja][ka].to_remove_if_last_after_add[na])break;ia[ja][ka].to_remove.push(na);}la.removeResources(ia[ja][ka].to_remove);});});}function z(ga,ha){var ia={};v.forEach(function(ja){ia[ja]={};u.forEach(function(ka){ia[ja][ka]=[];});});ha.forEach(function(ja){var ka=ba(ga,ja.thread_id),la=ca(ga,ja.thread_id);if(ka)la.forEach(function(ma){if(da(ga,ja.thread_id,ka,ma))ia[ka][ma].push(ja.thread_id);});});v.forEach(function(ja){u.forEach(function(ka){if(ia[ja][ka].length>0)aa(ga,ia[ja][ka],ja,ka);});});}function aa(ga,ha,ia,ja){ja=ja||i.ALL;ga._threadlistOrderMap[ia][ja].addResources(ha);v.forEach(function(ka){if(ka!=ia)ga._threadlistOrderMap[ka][ja].removeResources(ha);});}function ba(ga,ha){var ia=ga._threads.getThreadMetaNow(ha),ja=null;if(!ia){ja='No thread metadata';}else if(!ia.folder)ja='No thread folder';if(ja){var ka={error:ja,tid:ha};t.warn('no_thread_folder',ka);return null;}var la=ia.folder;if(ia.is_archived)la=m.ACTION_ARCHIVED;if(ga._threadlistOrderMap[la]){return la;}else return null;}function ca(ga,ha){var ia=ga._threads.getThreadMetaNow(ha),ja=[i.ALL];if(!ia){var ka={error:'no_thread_metadata',tid:ha};t.warn('no_thread_metadata',ka);return [];}if(ia.unread_count)ja.push(m.UNREAD);return ja;}function da(ga,ha,ia,ja){var ka=ga._threads.getThreadMetaNow(ha);return ka&&ka.timestamp&&ba(ga,ha)==ia&&r(ca(ga,ha),ja);}function ea(ga){this._fbid=ga;this._serverRequests=o.getForFBID(this._fbid);this._threadInformer=p.getForFBID(this._fbid);this._threads=q.getForFBID(this._fbid);this._threadlistOrderMap={};v.forEach(function(ha){this._threadlistOrderMap[ha]={};u.forEach(function(ia){this._threadlistOrderMap[ha][ia]=new n(function(ja){return this._threads.getThreadMetaNow(ja).timestamp;}.bind(this),function(ja,ka){return ka-ja;},this._serverRequests.canLinkExternally.bind(this._serverRequests));}.bind(this));}.bind(this));this._serverRequests.subscribe('update-threadlist',function(ha,ia){if(!fa(ia))return;var ja=ia.ordered_threadlists;if(ja&&ja.length){x(this,ja);}else{var ka=(ia.actions||[]).filter(function(la){return la.thread_id;});z(this,ia.threads||[]);y(this,ka);}this._threadInformer.updatedThreadlist();}.bind(this));}s(ea.prototype,{getThreadlist:function(ga,ha,ia,ja,ka,la){return this.getFilteredThreadlist(ga,ha,ia,i.ALL,ja,ka,la);},getFilteredThreadlist:function(ga,ha,ia,ja,ka,la,ma){var na=this._threadlistOrderMap[ia][ja],oa=na.executeOrEnqueue(ga,ha,ka,la),pa=na.getUnavailableResources(oa),qa=na.getError(ga,ha,la);if(pa.length||qa){if(na.getCurrentArraySize()<ga){var ra={start:ga,limit:ha,filter:ja,resources_size:na.getCurrentArraySize()};t.warn('range_with_gap',ra);}na.setError(false);this._serverRequests.fetchThreadlistInfo(na.getCurrentArraySize(),pa.length,ia,ja==i.ALL?null:ja,ma);}return oa;},getThreadlistUntilTimestamp:function(ga,ha,ia){ia=ia||i.ALL;return this._threadlistOrderMap[ha][ia].getElementsUntil(ga);},unsubscribe:function(ga,ha,ia){ia=ia||i.ALL;this._threadlistOrderMap[ha][ia].unsubscribe(ga);}});s(ea,l);function fa(ga){switch(ga.payload_source){case k.CLIENT_CHANGE_ARCHIVED_STATUS:case k.CLIENT_CHANGE_READ_STATUS:case k.CLIENT_CHANGE_FOLDER:case k.CLIENT_CHANNEL_MESSAGE:case k.CLIENT_DELETE_MESSAGES:case k.CLIENT_DELETE_THREAD:case k.CLIENT_SEND_MESSAGE:case k.SERVER_SEND_MESSAGE:case k.SERVER_CONFIRM_MESSAGES:case k.SERVER_FETCH_THREADLIST_INFO:case k.SERVER_THREAD_SYNC:return true;case k.SERVER_INITIAL_DATA:return ga.ordered_threadlists;default:return false;}}e.exports=ea;});
__d("MercuryChatUtils",["Env"],function(a,b,c,d,e,f){var g=b('Env'),h={canOpenChatTab:function(i){var j=i.is_canonical&&!i.is_canonical_user;return i.is_subscribed&&!j&&i.canonical_fbid!=g.user;}};e.exports=h;});
__d("ClickRefUtils",[],function(a,b,c,d,e,f){var g={get_intern_ref:function(h){if(!!h){var i={profile_minifeed:1,gb_content_and_toolbar:1,gb_muffin_area:1,ego:1,bookmarks_menu:1,jewelBoxNotif:1,jewelNotif:1,BeeperBox:1,navSearch:1};for(var j=h;j&&j!=document.body;j=j.parentNode){if(!j.id||typeof j.id!=='string')continue;if(j.id.substr(0,8)=='pagelet_')return j.id.substr(8);if(j.id.substr(0,8)=='box_app_')return j.id;if(i[j.id])return j.id;}}return '-';},get_href:function(h){var i=(h.getAttribute&&(h.getAttribute('ajaxify')||h.getAttribute('data-endpoint'))||h.action||h.href||h.name);return typeof i==='string'?i:null;},should_report:function(h,i){if(i=='FORCE')return true;if(i=='INDIRECT')return false;return h&&(g.get_href(h)||(h.getAttribute&&h.getAttribute('data-ft')));}};e.exports=g;});
__d("setUECookie",["Env"],function(a,b,c,d,e,f){var g=b('Env');function h(i){if(!g.no_cookies)document.cookie="act="+encodeURIComponent(i)+"; path=/; domain="+window.location.hostname.replace(/^.*(\.facebook\..*)$/i,'$1');}e.exports=h;});
__d("ClickRefLogger",["Arbiter","EagleEye","ClickRefUtils","collectDataAttributes","copyProperties","ge","setUECookie","$"],function(a,b,c,d,e,f){var g=b('Arbiter'),h=b('EagleEye'),i=b('ClickRefUtils'),j=b('collectDataAttributes'),k=b('copyProperties'),l=b('ge'),m=b('setUECookie'),n=b('$');function o(q){if(!l('content'))return [0,0,0,0];var r=n('content'),s=a.Vector2?a.Vector2.getEventPosition(q):{x:0,y:0};return [s.x,s.y,r.offsetLeft,r.clientWidth];}function p(q,r,event,s){var t=(!a.ArbiterMonitor)?'r':'a',u=[0,0,0,0],v,w,x;if(!!event){v=event.type;if(v=='click'&&l('content'))u=o(event);var y=0;event.ctrlKey&&(y+=1);event.shiftKey&&(y+=2);event.altKey&&(y+=4);event.metaKey&&(y+=8);if(y)v+=y;}if(!!r)w=i.get_href(r);var z=[];if(a.ArbiterMonitor){x=a.ArbiterMonitor.getInternRef(r);z=a.ArbiterMonitor.getActFields();}var aa=j(!!event?(event.target||event.srcElement):r,['ft','gt']);k(aa.ft,s.ft||{});k(aa.gt,s.gt||{});if(typeof(aa.ft.ei)==='string')delete aa.ft.ei;var ba=[q._ue_ts,q._ue_count,w||'-',q._context,v||'-',x||i.get_intern_ref(r),t,a.URI?a.URI.getRequestURI(true,true).getUnqualifiedURI().toString():location.pathname+location.search+location.hash,aa].concat(u).concat(z);return ba;}g.subscribe("ClickRefAction/new",function(q,r){if(i.should_report(r.node,r.mode)){var s=p(r.cfa,r.node,r.event,r.extra_data);m(r.cfa.ue);h.log('act',s);if(window.chromePerfExtension)window.postMessage({clickRef:JSON.stringify(s)},window.location.origin);}});});
__d("QuicklingAugmenter",["URI"],function(a,b,c,d,e,f){var g=b('URI'),h=[],i={addHandler:function(j){h.push(j);},augmentURI:function(j){var k=[],l=g(j);h.forEach(function(m){var n=m(l);if(!n)return l;k.push(m);l=l.addQueryData(n);});h=k;return l;}};e.exports=i;});
__d("Quickling",["AjaxPipeRequest","Arbiter","Class","CSSClassTransition","DocumentTitle","DOM","ErrorUtils","HTML","OnloadHooks","PageTransitions","QuicklingAugmenter","Run","URI","UserAgent","copyProperties","goOrReplace","isEmpty","QuicklingConfig"],function(a,b,c,d,e,f){var g=b('AjaxPipeRequest'),h=b('Arbiter'),i=b('Class'),j=b('CSSClassTransition'),k=b('DocumentTitle'),l=b('DOM'),m=b('ErrorUtils'),n=b('HTML'),o=b('OnloadHooks'),p=b('PageTransitions'),q=b('QuicklingAugmenter'),r=b('Run'),s=b('URI'),t=b('UserAgent'),u=b('copyProperties'),v=b('goOrReplace'),w=b('isEmpty'),x=b('QuicklingConfig'),y=x.version,z=x.sessionLength,aa=new RegExp(x.inactivePageRegex),ba=0,ca,da='',ea={isActive:function(){return true;},isPageActive:function(la){if(la=='#')return false;la=new s(la);if(la.getDomain()&&la.getDomain()!=s().getDomain())return false;if(la.getPath()=='/l.php'){var ma=la.getQueryData().u;if(ma){ma=s(unescape(ma)).getDomain();if(ma&&ma!=s().getDomain())return false;}}var na=la.getPath(),oa=la.getQueryData();if(!w(oa))na+='?'+s.implodeQuery(oa);return !aa.test(na);},getLoadCount:function(){return ba;}};function fa(la){la=la||'Facebook';k.set(la);if(t.ie()){da=la;if(!ca)ca=window.setInterval(function(){var ma=da,na=k.get();if(ma!=na)k.set(ma);},5000,false);}}function ga(la){var ma=document.getElementsByTagName('link');for(var na=0;na<ma.length;++na){if(ma[na].rel!='alternate')continue;l.remove(ma[na]);}if(la.length){var oa=l.find(document,'head');oa&&l.appendContent(oa,n(la[0]));}}function ha(la){var ma={version:y};this.parent.construct(this,la,{quickling:ma});}i.extend(ha,g);u(ha.prototype,{_preBootloadFirstResponse:function(la){return true;},_fireDomContentCallback:function(){this._request.cavalry&&this._request.cavalry.setTimeStamp('t_domcontent');p.transitionComplete();this._onPageDisplayed&&this._onPageDisplayed(this.pipe);this.parent._fireDomContentCallback();},_fireOnloadCallback:function(){if(this._request.cavalry){this._request.cavalry.setTimeStamp('t_hooks');this._request.cavalry.setTimeStamp('t_layout');this._request.cavalry.setTimeStamp('t_onload');}this.parent._fireOnloadCallback();},isPageActive:function(la){return ea.isPageActive(la);},_versionCheck:function(la){if(la.version==y)return true;var ma=['quickling','ajaxpipe','ajaxpipe_token'];v(window.location,s(la.uri).removeQueryData(ma),true);return false;},_processFirstResponse:function(la){var ma=la.getPayload();fa(ma.title);ga(ma.syndication||[]);window.scrollTo(0,0);j.go(document.body,ma.body_class||'',p.getMostRecentURI(),la.getRequest().getURI());h.inform('quickling/response');},getSanitizedParameters:function(){return ['quickling'];}});function ia(){ba++;return ba>=z;}function ja(la){g.setCurrentRequest(null);if(ia())return false;la=q.augmentURI(la);if(!ea.isPageActive(la))return false;window.ExitTime=Date.now();r.__removeHook('onafterloadhooks');r.__removeHook('onloadhooks');o.runHooks('onleavehooks');h.inform('onload/exit',true);new ha(la).setCanvasId('content').send();return true;}function ka(la){var ma=window[la];function na(oa,pa,qa){if(typeof oa!=='function')oa=eval.bind(null,oa);var ra=ma(m.guard(oa),pa);if(pa>0)if(qa!==false)r.onLeave(function(){clearInterval(ra);});return ra;}window[la]=na;}r.onAfterLoad(function la(){ka('setInterval');ka('setTimeout');p.registerHandler(ja,1);});e.exports=a.Quickling=ea;});
__d("ScubaSample",["Banzai","copyProperties"],function(a,b,c,d,e,f){var g="scuba_sample",h=b("Banzai"),i=b("copyProperties");function j(m,n,o){this.fields={};this.flush=function(){if(!m)return;var p={};i(p,this.fields);p._ds=m;if(n)p._lid=n;p._options=o;h.post(g,p);this.flush=function(){};this.flushed=true;};this.lid=n;return this;}function k(m,n,o){if(!this.fields[m])this.fields[m]={};this.fields[m][n]=o;return this;}function l(m){return function(n,o){if(this.flushed)return this;return k.call(this,m,n,o);};}i(j.prototype,{addNormal:l('normal'),addInteger:l('int'),addDenorm:l('denorm')});e.exports=j;});
__d("ScriptMonitorReporter",["ScriptMonitor","ScubaSample","setTimeoutAcrossTransitions","URI"],function(a,b,c,d,e,f){var g=b('ScriptMonitor'),h=b('ScubaSample'),i=b('setTimeoutAcrossTransitions'),j=b('URI');function k(o){var p=[];for(var q=0;q<o.length;q++)p.push(new RegExp(o[q],'i'));return p;}function l(o,p){for(var q=0;q<p.length;q++)if(p[q].src)o.push(p[q].src);}function m(o,p){for(var q=0;q<p.length;q++)if(p[q].test(o))return true;return false;}function n(o,p){var q=g.stop(),r={addGeoFields:1,addBrowserFields:1,addUser:1},s={};l(q,document.getElementsByTagName('script'));l(q,document.getElementsByTagName('iframe'));for(var t=0;t<q.length;t++){var u=q[t].replace(/\?.*/,''),v;if(s[u])continue;s[u]=1;if(!j.isValidURI(u)){v=true;}else if(m(u,p)){v=false;}else if(m(new j(u).getDomain(),o)){v=false;}else v=true;if(v)new h('unknown_scripts',0,r).addNormal('url',u).flush();}}e.exports={runScan:function(o,p){i(function(){n(k(o),k(p));},5000);}};});
__d("StringTransformations",[],function(a,b,c,d,e,f){e.exports={unicodeEscape:function(g){return g.replace(/[^A-Za-z0-9\-\.\:\_\$\/\+\=]/g,function(h){var i=h.charCodeAt().toString(16);return '\\u'+('0000'+i.toUpperCase()).slice(-4);});},unicodeUnescape:function(g){return g.replace(/(\\u[0-9A-Fa-f]{4})/g,function(h){return String.fromCharCode(parseInt(h.slice(2),16));});}};});
__d("UserActionHistory",["Arbiter","ClickRefUtils","ScriptPath","throttle"],function(a,b,c,d,e,f){var g=b('Arbiter'),h=b('ClickRefUtils'),i=b('ScriptPath'),j=b('throttle'),k={click:1,submit:1},l=false,m={log:[],len:0},n=j.acrossTransitions(function(){try{l._ua_log=JSON.stringify(m);}catch(q){l=false;}},1000);function o(){try{if(a.sessionStorage){l=a.sessionStorage;l._ua_log&&(m=JSON.parse(l._ua_log));}}catch(q){l=false;}m.log[m.len%10]={ts:Date.now(),path:'-',index:m.len,type:'init',iref:'-'};m.len++;g.subscribe("UserAction/new",function(r,s){var t=s.ua,u=s.node,event=s.event;if(!event||!(event.type in k))return;var v={path:i.getScriptPath(),type:event.type,ts:t._ue_ts,iref:h.get_intern_ref(u)||'-',index:m.len};m.log[m.len++%10]=v;l&&n();});}function p(){return m.log.sort(function(q,r){return (r.ts!=q.ts)?(r.ts-q.ts):(r.index-q.index);});}o();e.exports={getHistory:p};});
__d("clip",[],function(a,b,c,d,e,f){function g(h,i,j){return Math.min(Math.max(h,i),j);}e.exports=g;});
__d("BanzaiODS",["Banzai"],function(a,b,c,d,e,f){var g=b('Banzai');function h(){var j={};function k(l,m,n,o){var p=j[l]||(j[l]={}),q=p[m]||(p[m]=[0]);if(n===undefined)n=1;if(o===undefined)o=1;n=Number(n);o=Number(o);if(!isFinite(n)||!isFinite(o))return;q[0]+=n;if(arguments.length>=4){if(!q[1])q[1]=0;q[1]+=o;}}return {bumpEntityKey:function(l,m,n){k(l,m,n);},bumpFraction:function(l,m,n,o){k(l,m,n,o);},flush:function(l){for(var m in j)g.post('ods:'+m,j[m]);j={};}};}var i=h();i.create=h;g.subscribe(g.SEND,i.flush);e.exports=i;});
__d("MusicButtonStore",[],function(a,b,c,d,e,f){var g={},h={addButton:function(i,j){g[i]=j;return j;},getButton:function(i){return g[i];},getButtons:function(){return g;},removeButton:function(i){g[i]&&g[i].resetLoadingTimers();delete g[i];}};e.exports=h;});
__d("MusicConstants",["URI"],function(a,b,c,d,e,f){var g=b('URI'),h={DEBUG:false,LIVE_LISTEN_MIN_SPOTIFY_VERSION:'spotify-0.6.6.0.g5a9eaca5',enableDebug:function(){this.DEBUG=true;},sameURLs:function(i,j){var k=/\/$/;if(i&&j){i=g(i);j=g(j);return i.getDomain()==j.getDomain()&&i.getPath()==j.getPath();}return false;},greaterOrEqualToMinimumVersion:function(i,j){var k=/(?:\d+\.)+/,l=i.match(k)[0].split('.').slice(0,-1),m=j.match(k)[0].split('.').slice(0,-1);if(l.length!==m.length)return false;for(var n=0;n<m.length;n++)if(+l[n]<+m[n]){return false;}else if(+l[n]>+m[n])return true;return true;},sanitizeForProviders:function(i){var j={};for(var k in i)if(this.ALLOWED_EXTERNAL_CONTEXT_PARAMS[k])j[k]=i[k];return j;},OP:{RESUME:'RESUME',PAUSE:'PAUSE',PLAY:'PLAY',VERSION:'VERSION'},STATUS_CHANGE_OP:{STATUS:'STATUS',LOGIN:'LOGIN',REINFORM:'REINFORM'},STATUS_CHANGE_EVENT:{playing:'PLAY_STATE_CHANGED',track:'TRACK_CHANGED'},DIAGNOSTIC_EVENT:{ALL_PAUSED:'ALL_PAUSED',ALL_OFFLINE:'ALL_OFFLINE',OFFLINE:'OFFLINE',ONLINE:'ONLINE',SEARCHING:'SEARCHING',HIT:'HIT',MISS:'MISS',RESIGN:'RESIGN',IFRAME_POLLING:'IFRAME_POLLING',RELAUNCH:'RELAUNCH',STATE_CHANGE:'STATE_CHANGE',WRONG_VERSION:'WRONG_VERSION',SERVICE_ERROR:'SERVICE_ERROR',INCORRECT_ONLINE_STATE:'INCORRECT_ONLINE_STATE',LOG_SEND_OP:'LOG_SEND_OP',REQUEUE_OP:'REQUEUE_OP'},ALLOWED_STATUS_PARAMS:{playing:'playing',track:'track',context:'context',client_version:'client_version',start_time:'start_time',expires_in:'expires_in',open_graph_state:'open_graph_state'},ALLOWED_EXTERNAL_CONTEXT_PARAMS:{uri:true,song:true,radio_station:true,album:true,playlist:true,musician:true,song_list:true,offset:true,title:true,request_id:true,listen_with_friends:true,needs_tos:true},LIVE_LISTEN_OP:{NOW_LEADING:'NOW_LEADING',NOW_LISTENING:'NOW_LISTENING',END_SESSION:'END_SESSION',SONG_PLAYING:'SONG_PLAYING',LISTENER_UPDATE:'LISTENER_UPDATE',QUEUE_SESSION:'QUEUE_SESSION',PLAY_ERROR:'PLAY_ERROR',SESSION_UPDATED:'SESSION_UPDATED',QUEUING_SESSION:'QUEUING_SESSION'},MUSIC_BUTTON:{ACTIVATE:'ACTIVATE'},ERROR:{1:'SERVICE_UNAVAILABLE_WITHOUT_PREMIUM',2:'SERVICE_UNAVAILABLE_WITHOUT_PREMIUM_OR_WAIT',3:'SERVICE_UNAVAILABLE_BILLING_ISSUE',4:'SERVICE_UNAVAILABLE_TECHNICAL_ISSUE',5:'AUDIO_AD_PLAYING',99:'SERVICE_TEMPORARILY_UNAVAILABLE',101:'SONG_UNAVAILABLE_WITHOUT_PURCHASE',102:'SONG_UNAVAILABLE_WITHOUT_PREMIUM',103:'SONG_UNAVAILABLE_INDEFINITELY'}};e.exports=a.MusicConstants||h;});
__d("MusicEvents",["Arbiter"],function(a,b,c,d,e,f){var g=b('Arbiter');e.exports=a.MusicEvents=new g();});
__d("MusicButton",["BanzaiODS","Bootloader","copyProperties","CSS","DOM","MusicButtonStore","MusicConstants","MusicEvents","Parent","ScubaSample","Tooltip","cx"],function(a,b,c,d,e,f){var g=b('BanzaiODS'),h=b('Bootloader'),i=b('copyProperties'),j=b('CSS'),k=b('DOM'),l=b('MusicButtonStore'),m=b('MusicConstants'),n=b('MusicEvents'),o=b('Parent'),p=b('ScubaSample'),q=b('Tooltip'),r=b('cx'),s=function(t,u,v,w,x,y){this.provider=t;this.buttonElem=u;this.url=v;this.context=w||{};this.mediaType=x;this.setState(this.STATES.OFFLINE);this.tooltip=y||'';n.subscribe(m.MUSIC_BUTTON.ACTIVATE,this.processClick.bind(this));};i(s,{tracksetableTypes:[]});i(s.prototype,{SHOW_LOADING_TIMEOUT:500,HIDE_LOADING_TIMEOUT:4000,RECENTLY_ONLINE_TIMEOUT:6000,STATES:{PLAYING:'music_playing',PAUSED:'music_paused',LOADING:'music_loading',DISABLED:'music_disabled',OFFLINE:'music_offline'},setState:function(t){if(t!==this.STATES.LOADING){this.resetLoadingTimers();this.previousState=this.state||t;}if(t===this.STATES.PLAYING){q.set(this.buttonElem,this.tooltip);}else q.set(this.buttonElem,'');var u=this.buttonElem.parentNode;this.state&&j.removeClass(u,this.state);this.state=t;j.addClass(u,this.state);},isTracksetable:function(t){return s.tracksetableTypes.indexOf(this.mediaType)!==-1;},handleIncomingEvent:function(t,u){clearTimeout(this._showLoadingTimer);if(u&&u.provider&&u.provider!=this.provider)return;switch(t){case m.DIAGNOSTIC_EVENT.ONLINE:case m.STATUS_CHANGE_EVENT.track:case m.STATUS_CHANGE_EVENT.playing:var v=u&&u.track&&u.track.uri,w=u&&u.context&&u.context.uri;if(u&&u.playing&&(m.sameURLs(v,this.url)||m.sameURLs(w,this.url))){this.setState(this.STATES.PLAYING);}else if(this.state===this.STATES.LOADING&&(this.previousState===this.STATES.PAUSED||this.previousState===this.STATES.OFFLINE)){clearTimeout(this._attemptingPlayTimer);this._attemptingPlayTimer=this.setState.bind(this,this.STATES.PAUSED).defer(this.RECENTLY_ONLINE_TIMEOUT,false);}else if(!this._attemptingPlayTimer)this.setState(this.STATES.PAUSED);break;case m.DIAGNOSTIC_EVENT.OFFLINE:this.setState(this.STATES.OFFLINE);break;case m.DIAGNOSTIC_EVENT.ALL_OFFLINE:this.setState(this.STATES.OFFLINE);break;}},processClick:function(t,u){if(u!=this.buttonElem){if(this.state===this.STATES.LOADING)this.previousState&&this.setState(this.previousState);return;}var v=new p('music_play_button_click',null,{addBrowserFields:true,addGeoFields:true,addUser:true});v.addNormal('uses_bridge','1');v.addNormal('state',this.state);v.addNormal('provider',this.provider);v.addNormal('class','MusicButton');v.addDenorm('url',this.url);v.flush();if(this.state!=this.STATES.PLAYING){g.bumpEntityKey('music_play_button','music_play_button_click');g.bumpEntityKey('music_play_button','music_play_button_click.'+this.provider);var w=o.byClass(this.buttonElem,"_4--s");if(w){j.addClass(w,"_4--t");j.removeClass.curry(w,"_4--t").defer(3000);}}var x=this.isTracksetable()&&o.byClass(this.buttonElem,'music_trackset_container'),y=[];if(x){var z=x.getAttribute('data-trackset-title'),aa=this.provider,ba=k.scry(x,'.music_button');for(var ca=0;ca<ba.length;ca++){var da=l.getButton([ba[ca].id]);if(da&&da.provider==aa&&da.isTracksetable())y.push(da.url);}}if(!a.Music)this.showLoading(true);h.loadModules(['Music'],function(ea){var fa=(x&&y.length>1)?ea.playPauseSongList(this.provider,this.url,y,z,this.context):ea.playPauseSong(this.provider,this.url,this.context);this.showLoading(!fa);}.bind(this));},showLoading:function(t){this.resetLoadingTimers();this._hideLoadingTimer=this._timeout.bind(this,t).defer(this.HIDE_LOADING_TIMEOUT,false);this._showLoadingTimer=this.setState.bind(this,this.STATES.LOADING).defer(this.SHOW_LOADING_TIMEOUT,false);},resetLoadingTimers:function(){clearTimeout(this._hideLoadingTimer);clearTimeout(this._showLoadingTimer);clearTimeout(this._attemptingPlayTimer);this._attemptingPlayTimer=null;},destroy:function(){this.resetLoadingTimers();this.buttonElem=null;},_timeout:function(t){a.Music&&a.Music.reInform([this.provider]);if(!t&&this.state===this.STATES.LOADING)this.setState(this.STATES.PAUSED);}});e.exports=s;});
__d("MusicButtonManager",["Event","DOM","KeyedCallbackManager","Layer","MusicButton","MusicButtonStore","MusicConstants","MusicEvents","Parent","$","copyProperties","ge"],function(a,b,c,d,e,f){var g=b('Event'),h=b('DOM'),i=b('KeyedCallbackManager'),j=b('Layer'),k=b('MusicButton'),l=b('MusicButtonStore'),m=b('MusicConstants'),n=b('MusicEvents'),o=b('Parent'),p=b('$'),q=b('copyProperties'),r=b('ge'),s=new i(),t=null,u={},v=0;function w(da){var ea=da.getTarget(),fa=o.byClass(ea,'music_button');fa=fa||(!(da.getModifiers&&da.getModifiers().any)&&x(ea));if(!fa)return;return y(fa,da);}function x(da){var ea=o.byClass(da,'music_button_trigger')&&o.byClass(da,'music_button_trigger_group');if(ea){var fa=h.scry(ea,'.music_button');if(fa.length)return fa[0];}return null;}function y(da,event){event&&event.stop();n.inform(m.MUSIC_BUTTON.ACTIVATE,da);return false;}function z(da){a.Music&&a.Music.reInform(da);}function aa(da,ea){var fa=l.getButtons();for(var ga in fa)if(fa[ga].noGC||r(ga)){fa[ga].handleIncomingEvent(da,ea);}else l.removeButton(ga);}var ba={init:function(da){if(t)return;t=true;k.tracksetableTypes=da||[];g.listen(document.body,'click',w);n.subscribe([m.STATUS_CHANGE_EVENT.playing,m.STATUS_CHANGE_EVENT.track,m.DIAGNOSTIC_EVENT.OFFLINE,m.DIAGNOSTIC_EVENT.ALL_OFFLINE,m.DIAGNOSTIC_EVENT.ONLINE],aa);},add:function(da,ea,fa,ga,ha,ia){t||ba.init();var ja=ea.id,ka=l.getButton(ja);if(ka)return ka;ka=l.addButton(ja,new k(da,ea,fa,q({button_id:ja},ga),ha,ia));var la=o.byClass(ea,'uiOverlay');if(la){ka.noGC=true;var ma=j.subscribe('destroy',function(na,oa){if(h.contains(oa.getRoot(),ea)){l.removeButton(ja);j.unsubscribe(ma);}});}if(da&&!u[da])u[da]=function(){var na=Object.keys(u);na.length&&z(na);u={};}.defer();return ka;},addButton:function(da,ea,fa,ga,ha,ia){if(!r(ea))return;var ja=p(ea);return ba.add(da,ja,fa,ga,ha,ia);},asyncAddMusicButton:function(da,ea){da.setAttribute('id','music_button_'+v++);ca(da,ea);},tryAddButtonInDOM:function(da,ea){var fa=r(da);fa&&ca(fa,ea);},addMusicData:function(da,ea,fa,ga,ha,ia){s.setResource(da,{provider:ea,uri:fa,context:ga,media_type:ha,tooltip:ia});}};function ca(da,ea){var fa=h.find(da,'a.button_anchor').getAttribute('href');s.executeOrEnqueue(fa,function(ga){return ba.add(ga.provider,da,ga.uri,ga.context,ga.media_type,ea?ga.tooltip:'');});}e.exports=a.MusicButtonManager||ba;});
__d("TinyViewport",["Arbiter","CSS","DOM","Event","queryThenMutateDOM"],function(a,b,c,d,e,f){var g=b('Arbiter'),h=b('CSS'),i=b('DOM'),j=b('Event'),k=b('queryThenMutateDOM'),l=document.documentElement,m,n,o=k.bind(null,function(){n=n||i.getDocumentScrollElement();m=l.clientHeight<400||l.clientWidth<n.scrollWidth;},function(){h.conditionClass(l,'tinyViewport',m);h.conditionClass(l,'canHaveFixedElements',!m);},'TinyViewport');o();g.subscribe('quickling/response',o);j.listen(window,'resize',o);});
__d("WebStorageMonster",["Event","AsyncRequest","UserActivity","StringTransformations","arrayContains","setTimeoutAcrossTransitions"],function(a,b,c,d,e,f){var g=b('Event'),h=b('AsyncRequest'),i=b('UserActivity'),j=b('StringTransformations'),k=b('arrayContains'),l=b('setTimeoutAcrossTransitions'),m=300000,n=false;function o(t){var u={};for(var v in t){var w=t.getItem(v),x=j.unicodeEscape(v);if(typeof w==='string')u[x]=w.length;}return u;}function p(t){if(a.localStorage&&t.keys)s._getLocalStorageKeys().forEach(function(u){if(k(t.keys,u))a.localStorage.removeItem(u);});}function q(t){if(a.localStorage)s._getLocalStorageKeys().forEach(function(u){if(!t.some(function(v){return new RegExp(v).test(u);}))a.localStorage.removeItem(u);});if(a.sessionStorage)a.sessionStorage.clear();}function r(t){if(i.isActive(m)){l(r.curry(t),m);}else s.cleanNow(t);}var s={registerLogoutForm:function(t,u){g.listen(t,'submit',function(v){s.cleanOnLogout(u);});},schedule:function(t){if(n)return;n=true;r(t);},cleanNow:function(t){var u=Date.now(),v={},w=false;['localStorage','sessionStorage'].forEach(function(y){if(a[y]){v[y]=o(a[y]);w=true;}});var x=Date.now();v.logtime=x-u;if(w)new h('/ajax/webstorage/process_keys.php').setData(v).setHandler(function(y){if(!t){var z=y.getPayload();if(z.keys)z.keys=z.keys.map(j.unicodeUnescape);p(z);}}.bind(this)).send();},cleanOnLogout:function(t){if(t)q(t);if(a.sessionStorage)a.sessionStorage.clear();},_getLocalStorageKeys:Object.keys.curry(a.localStorage)};e.exports=s;});