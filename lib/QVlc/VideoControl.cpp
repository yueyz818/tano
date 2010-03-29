/****************************************************************************
* QVlc - Qt and libVLC connector library
* VideoControl.h: Video and subtitles controller
*****************************************************************************
* Copyright (C) 2008-2010 Tadej Novak
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* This file may be used under the terms of the
* GNU General Public License version 3.0 as published by the
* Free Software Foundation and appearing in the file LICENSE.GPL
* included in the packaging of this file.
*****************************************************************************/

#include "Instance.h"
#include "VideoControl.h"

QVlc::VideoControl::VideoControl(const QString &lang, QObject *parent)
	: QObject(parent), _actionSubList(QList<QAction*>()), _mapSub(QMap<QString,int>()), _actionSubGroup(0),
	_actionVideoList(QList<QAction*>()), _mapVideo(QMap<QString,int>()), _actionVideoGroup(0),
	_manualLanguage(false), _preferedLanguage(lang.split(" / "))
{
	_timer = new QTimer(this);
	connect(_timer, SIGNAL(timeout()), this, SLOT(updateSubtitleActions()));
	connect(_timer, SIGNAL(timeout()), this, SLOT(updateVideoActions()));

	_timer->start(2000);
}

QVlc::VideoControl::~VideoControl()
{
	delete _timer;

	for(int i=0; i<_actionSubList.size(); i++)
		delete _actionSubList[i];
	for(int i=0; i<_actionVideoList.size(); i++)
		delete _actionVideoList[i];
	if(_actionSubGroup)
		delete _actionSubGroup;
	if(_actionVideoGroup)
		delete _actionVideoGroup;
}

void QVlc::VideoControl::updateSubtitleActions() {
	for(int i=0; i<_actionSubList.size(); i++)
		delete _actionSubList[i];
	_actionSubList.clear();
	_mapSub.clear();

	if(_actionSubGroup)
		delete _actionSubGroup;
	_actionSubGroup = new QActionGroup(this);

	if(!Instance::isActive()) {
		emit actions("sub", _actionSubList);
		return;
	}

#if VLC_1_1
	if(libvlc_video_get_spu_count(_vlcCurrentMediaPlayer) != 0) {
		libvlc_track_description_t *desc;
		desc = libvlc_video_get_spu_description(_vlcCurrentMediaPlayer);
#else
	if(libvlc_video_get_spu_count(_vlcCurrentMediaPlayer, _vlcException) != 0) {
		libvlc_track_description_t *desc;
		desc = libvlc_video_get_spu_description(_vlcCurrentMediaPlayer, _vlcException);
#endif
		_mapSub.insert(QString().fromUtf8(desc->psz_name), 0);
		_actionSubList << new QAction(QString().fromUtf8(desc->psz_name), this);
#if VLC_1_1
		if(libvlc_video_get_spu_count(_vlcCurrentMediaPlayer) > 1) {
			for(int i = 1; i < libvlc_video_get_spu_count(_vlcCurrentMediaPlayer); i++) {
				desc = desc->p_next;
				_mapSub.insert(QString().fromUtf8(desc->psz_name), i);
				_actionSubList << new QAction(QString().fromUtf8(desc->psz_name), this);
			}
		}
#else
		if(libvlc_video_get_spu_count(_vlcCurrentMediaPlayer, _vlcException) > 1) {
			for(int i = 1; i < libvlc_video_get_spu_count(_vlcCurrentMediaPlayer, _vlcException); i++) {
				desc = desc->p_next;
				_mapSub.insert(QString().fromUtf8(desc->psz_name), i);
				_actionSubList << new QAction(QString().fromUtf8(desc->psz_name), this);
			}
		}
#endif
	} else {
		emit actions("sub", _actionSubList);
		return;
	}

	Instance::checkError();

	for (int i = 0; i < _actionSubList.size(); ++i) {
		_actionSubList[i]->setCheckable(true);
		_actionSubGroup->addAction(_actionSubList[i]);
		connect(_actionSubList[i], SIGNAL(triggered()), this, SLOT(updateSubtitles()));

		if(!_manualLanguage)
			for(int j = 0; j < _preferedLanguage.size(); ++j)
				if(_actionSubList[i]->text().contains(_preferedLanguage[j],Qt::CaseInsensitive)) {
					_actionSubList[i]->trigger();
					_manualLanguage = true;
				}
	}
#if VLC_1_1
	_actionSubList[libvlc_video_get_spu(_vlcCurrentMediaPlayer)]->setChecked(true);
#else
	_actionSubList[libvlc_video_get_spu(_vlcCurrentMediaPlayer, _vlcException)]->setChecked(true);
#endif
	Instance::checkError();

	emit actions("sub", _actionSubList);

	_timer->start(60000);
}

void QVlc::VideoControl::updateSubtitles()
{
	int id = _mapSub.value(_actionSubGroup->checkedAction()->text());

#if VLC_1_1
	libvlc_video_set_spu(_vlcCurrentMediaPlayer, id);
#else
	libvlc_video_set_spu(_vlcCurrentMediaPlayer, id ,_vlcException);
#endif

	Instance::checkError();
}

void QVlc::VideoControl::loadSubtitle(const QString &sub)
{
	if(!Instance::isActive() || sub.isEmpty())
		return;

#if VLC_1_1
	libvlc_video_set_subtitle_file(_vlcCurrentMediaPlayer, sub.toUtf8().data());
#else
	libvlc_video_set_subtitle_file(_vlcCurrentMediaPlayer, sub.toUtf8().data(), _vlcException);
#endif

	Instance::checkError();

	_timer->start(1000);
}

void QVlc::VideoControl::updateVideoActions() {
	for(int i=0; i<_actionVideoList.size(); i++)
		delete _actionVideoList[i];
	_actionVideoList.clear();
	_mapVideo.clear();

	if(_actionVideoGroup)
		delete _actionVideoGroup;
	_actionVideoGroup = new QActionGroup(this);

	if(!Instance::isActive()) {
		emit actions("video", _actionVideoList);
		return;
	}

#if VLC_1_1
	if(libvlc_video_get_track_count(_vlcCurrentMediaPlayer) != 0) {
		libvlc_track_description_t *desc;
		desc = libvlc_video_get_track_description(_vlcCurrentMediaPlayer);
#else
	if(libvlc_video_get_track_count(_vlcCurrentMediaPlayer, _vlcException) != 0) {
		libvlc_track_description_t *desc;
		desc = libvlc_video_get_track_description(_vlcCurrentMediaPlayer, _vlcException);
#endif
		_mapVideo.insert(QString().fromUtf8(desc->psz_name), desc->i_id);
		_actionVideoList << new QAction(QString().fromUtf8(desc->psz_name), this);
#if VLC_1_1
		if(libvlc_video_get_track_count(_vlcCurrentMediaPlayer) > 1) {
			for(int i = 1; i < libvlc_video_get_track_count(_vlcCurrentMediaPlayer); i++) {
				desc = desc->p_next;
				_mapVideo.insert(QString().fromUtf8(desc->psz_name), desc->i_id);
				_actionVideoList << new QAction(QString().fromUtf8(desc->psz_name), this);
			}
		}
#else
		if(libvlc_video_get_track_count(_vlcCurrentMediaPlayer, _vlcException) > 1) {
			for(int i = 1; i < libvlc_video_get_track_count(_vlcCurrentMediaPlayer, _vlcException); i++) {
				desc = desc->p_next;
				_mapVideo.insert(QString().fromUtf8(desc->psz_name), desc->i_id);
				_actionVideoList << new QAction(QString().fromUtf8(desc->psz_name), this);
			}
		}
#endif
	} else {
		emit actions("video", _actionVideoList);
		return;
	}

	Instance::checkError();

	for (int i = 0; i < _actionVideoList.size(); ++i) {
		_actionVideoList[i]->setCheckable(true);
		_actionVideoGroup->addAction(_actionVideoList[i]);
		connect(_actionVideoList[i], SIGNAL(triggered()), this, SLOT(updateVideo()));
	}
#if VLC_1_1
	_actionVideoList[libvlc_video_get_track(_vlcCurrentMediaPlayer)]->setChecked(true);
#else
	_actionVideoList[libvlc_video_get_track(_vlcCurrentMediaPlayer, _vlcException)]->setChecked(true);
#endif
	Instance::checkError();

	emit actions("video", _actionVideoList);

	_timer->start(60000);
}

void QVlc::VideoControl::updateVideo()
{
	int id = _mapVideo.value(_actionVideoGroup->checkedAction()->text());

#if VLC_1_1
	libvlc_video_set_track(_vlcCurrentMediaPlayer, id);
#else
	libvlc_video_set_track(_vlcCurrentMediaPlayer, id ,_vlcException);
#endif

	Instance::checkError();
}

void QVlc::VideoControl::mediaChange()
{
	_timer->start(2000);
	_manualLanguage = false;
}

void QVlc::VideoControl::setDefaultSubtitleLanguage(const QString &lang)
{
	_preferedLanguage = lang.split(" / ");
}
