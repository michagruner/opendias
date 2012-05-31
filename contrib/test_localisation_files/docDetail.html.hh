
<script type="text/javascript" src="/opendias/includes/jquery-ui.min.js"></script>
<script type="text/javascript" src="/opendias/includes/jquery.mousewheel.js"></script>
<script type="text/javascript" src="/opendias/includes/jquery.gzoom.js"></script>
<script type="text/javascript" src="/opendias/includes/jquery.easySlider.js"></script>
<script type="text/javascript" src="/opendias/includes/jquery.tagsinput.js"></script>
<link rel="stylesheet" href="/opendias/style/jquery-ui.css" type="text/css" media="screen" /> 
<link rel="stylesheet" href="/opendias/style/jquery.gzoom.css" type="text/css" media="screen" /> 
<link rel="stylesheet" href="/opendias/style/jquery.easyslider.css" type="text/css" />
<link rel="stylesheet" href="/opendias/style/jquery.tagsinput.css" type="text/css" />
<style>
.ui-datepicker { font-size: 11px; }
.ui-autocomplete { font-size: 11px; max-height: 150px; overflow-y: auto; }
</style>

<h2>######## ######</h2>

<div id='saveAlert'>##### ##</div>

<div id='leftPane'>
  <div class='picBorder'>
    <div id="slider">
      <ul>
      </ul>
    </div>
    <div class='cclear'></div>
  </div>
</div>

<div id='rightPane'>
  <div class='row'>
    <div class='label'>### ##:</div>
    <div class='data'>
      <div id='docid' class='innerLeftDisplay'>&nbsp;</div>
      <div class='innerRightDisplay'>
        <button id='delete'>######</button>
      </div>
    </div>
  </div>

  <div class='row'>
    <div class='label'>#### ####:</div>
    <div id='scanDate' class='data'></div>
  </div>

  <div class='row'>
    <div class='label'>### ####:</div>
    <div id='type' class='data'></div>
  </div>

  <div class='row'>
    <div class='label'>###### ########:</div>
    <div class='data'><input id='actionrequired' type='checkbox' /></div>
  </div>

  <div class='row'>
    <div class='label'>######## ####:</div>
    <div class='data'><input id='hardcopyKept' type='checkbox' /></div>
  </div>

  <div class='row'>
    <div class='label'>#####:</div>
    <div class='data'><input id='title' size=40 /></div>
  </div>

  <div class='row'>
    <div class='label'>######## ####:</div>
    <div class='data'><input id='docDate' /></div>
  </div>

  <div class='row '>
    <div class='label'>######### ####:</div>
    <div class='data'>
      <textarea id='ocrtext' rows=2 cols=40></textarea>
    </div>
  </div>

  <div class='row'>
    <div class='label'>######## ####:</div>
    <div class='data'>
      <input id='tags' type='text' class='tags'>
    </div>
  </div>

  <div class='row'>
    <div class='label'>###### ####:</div>
    <div class='data divider'>
      <div id="doclinks_tagsinput" class="tagsinput" style="width: 300px; ">
        <div id="doclinks_addTag"><input id="doclinks_tag" value="" data-default="#### ## ###" style="color: rgb(102, 102, 102); " class="ui-autocomplete-input" autocomplete="off" role="textbox" aria-autocomplete="list" aria-haspopup="true"></div>
        <div class="tags_clear"></div>
      </div>
    </div>
  </div>

</div>

<script type="text/javascript" src="/opendias/includes/local/generic.resource"></script>
<script type="text/javascript" src="/opendias/includes/local/openDias.saveDetails.js.resource"></script>
<script type="text/javascript" src="/opendias/includes/openDias.saveDetails.js"></script>
<script type="text/javascript" src="/opendias/includes/local/openDias.loadDetails.js.resource"></script>
<script type="text/javascript" src="/opendias/includes/openDias.loadDetails.js"></script>
